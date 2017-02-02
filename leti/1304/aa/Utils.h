#pragma once

std::tuple<unsigned char, int, int, std::string, int, int> parse(int argc, const char* argv[])
{
	unsigned char brightnessValue = 128;
	int maxParallel = 10;
	int countMatrix = 20;
	std::string logFile = "./log.txt";
	int width = 320;
	int height = 240;

	for (int i = 1; i < argc; i += 2) {
		if (!std::strcmp(argv[i], "-b")) {
			brightnessValue = atoi(argv[i + 1]);
		} else if (!std::strcmp(argv[i], "-l")) {
			maxParallel = atoi(argv[i + 1]);
		} else if(!std::strcmp(argv[i], "-c")) {
			countMatrix = atoi(argv[i + 1]);
		} else if (!std::strcmp(argv[i], "-f")) {
			logFile = argv[i + 1];
		} else if (!std::strcmp(argv[i], "-w")) {
			width = atoi(argv[i + 1]);
		} else if (!std::strcmp(argv[i], "-h")) {
			height = atoi(argv[i + 1]);
		}
	}

	return std::tuple<unsigned char, int, int, std::string, int, int> {brightnessValue, maxParallel, countMatrix, logFile, width, height};
}
