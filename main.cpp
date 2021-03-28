#include "opencv2/opencv.hpp"
#include "sapi.h"
#include <string>
#include <iostream>
#include <mutex>
#include <thread>

static ISpVoice* pVoice = NULL;

void initializeSAPI()
{
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr))//S_FALSE is acceptable(*)
	{
		return;
	}
	CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
}

void speakWstring(std::wstring text)
{
	if (NULL == pVoice)
		initializeSAPI();
	if (NULL == pVoice)
		return;
	text = L"<voice required='Gender = Female;'>" + text;
	pVoice->Speak(text.c_str(), 0, NULL);
}

std::mutex m1;

void detectQrAndSpeak(const cv::InputArray& frame)
{
	std::lock_guard<std::mutex> lckGrd(m1);
	cv::QRCodeDetector qrDetector;
	auto qrStringData = std::string();
	cv::Mat points;
	if (qrDetector.detect(frame, points) && !points.empty())
	{
		cv::Mat straightQrImg;
		qrStringData = qrDetector.decode(frame, points, straightQrImg);
		if (qrStringData.length() > 0)
		{
			speakWstring(L"QrDetected");
			std::cout << qrStringData << std::endl;
		}
	}
}

int main()
{
	cv::Mat frame;
	cv::VideoCapture wbcam(0);


	if (!wbcam.isOpened())
		return 1;

	while (wbcam.read(frame))
	{
		if (frame.empty())
		{
			break;
		}
		std::thread t1(detectQrAndSpeak, frame);//avoid detection delay
		cv::imshow("webcam", frame);
		if (cv::waitKey(10) == 27)//if pressed esc key
		{
			t1.join();
			break; // escape loop
		}
		t1.detach();
	}
	// Clear Everything
	if (NULL != pVoice)
	{
		pVoice->Release();
		pVoice = NULL;
	}
	cv::destroyAllWindows();

	return 0;
}
