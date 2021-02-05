#pragma once

#include <filesystem>

#include "itkVersion.h"
#include "itkGPUImage.h"
#include "itkImage.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageFileReader.h"
#include "itkImageSeriesReader.h"
#include "itkNumericSeriesFileNames.h"

using PixelType = signed short;
constexpr unsigned int Dimension = 3;

using ImageType = itk::Image<PixelType, Dimension>;
using ReaderType = itk::ImageSeriesReader<ImageType>;
using ImageIOType = itk::GDCMImageIO;
using DictionaryType = itk::MetaDataDictionary;
using MetaDataStringType = itk::MetaDataObject<std::string>;

namespace ITKReader {
	static std::vector<std::string> fileNames = {};
	static std::vector<std::string> filePaths = {};
	//static std::vector<std::pair<std::string, std::string>> files = {};
	static bool filesLoaded = false;
	static ImageType::Pointer images = ImageType::New();

	void loadFiles(std::vector<std::pair<std::string, std::string>>& selected);

	// maybe make it bool
	void loadDirectory(std::string path);

	// (Re)reads in all Dicom files
	void readFiles();

	// Data
	bool hasFiles();
	int numFiles();

	std::string fileNameAt(int idx);
	std::string filePathAt(int idx);

	float getAlphaAt(int x, int y, int z);
}