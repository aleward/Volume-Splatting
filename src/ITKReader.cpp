#include "ITKReader.hpp"


void ITKReader::loadFiles(std::vector<std::pair<std::string, std::string>>& selected) {
	fileNames.clear();
	filePaths.clear();
	//files = selected;//std::move(selected);

	std::transform(selected.begin(), selected.end(), std::back_inserter(fileNames),
		std::mem_fn(&std::pair<std::string, std::string>::first));
	std::transform(selected.begin(), selected.end(), std::back_inserter(filePaths),
		std::mem_fn(&std::pair<std::string, std::string>::second));

	filesLoaded = !filePaths.empty();
	readFiles();
}

void ITKReader::loadDirectory(std::string path) {
	fileNames.clear();
	filePaths.clear();
	//files.clear();
	std::string dcmStr = ".dcm";
	// Todo - try doing this with CUDA

	// TODO - adapt - cutting it off at 512 for now
	int count = 0;

	for (const auto & file : std::filesystem::directory_iterator(path)) {
		if (count == 512) { break; }
		count++;

		std::filesystem::path currFile = file.path();
		
		// Only add the DICOM files
		if (currFile.extension().compare(dcmStr) == 0) {
			fileNames.push_back(currFile.filename().string());
			filePaths.push_back(currFile.string());

			//files.push_back(std::make_pair<std::string, std::string>(
			//	currFile.filename().string(), currFile.string()));
		}
	}

	filesLoaded = !filePaths.empty();
	readFiles();
}

void ITKReader::readFiles() {
	if (filesLoaded) {
		//images->Delete();
		// Setup the reader
		ReaderType::Pointer reader = ReaderType::New();
		ImageIOType::Pointer dicomIO = ImageIOType::New(); //(I think the images are z,y,x - NO call the Index type for xyz)
		reader->SetImageIO(dicomIO);

		reader->SetFileNames(filePaths);

		// Trigger the reader
		try {
			// TODO - takes a MASSIVELY long time. Enable ITK GPU
			reader->Update();
		}
		catch (itk::ExceptionObject& e) {
			filesLoaded = false;
			fileNames.clear();
			filePaths.clear();
			std::cout << e << std::endl;
			return;
		}

		images = ImageType::New();
		images = reader->GetOutput();
		//// Get all DICOM tags of type string - to use to confirm distance between scans
		//const DictionaryType & dictionary = dicomIO->GetMetaDataDictionary();
		//auto itr = dictionary.Begin();
		//auto end = dictionary.End();

	}
}

bool ITKReader::hasFiles() {
	return filesLoaded;
}

int ITKReader::numFiles() {
	return filePaths.size();
	//return files.size();
}

std::string ITKReader::fileNameAt(int idx) {
	return fileNames.at(idx);
	//return files.at(idx).first;
}

std::string ITKReader::filePathAt(int idx) {
	return filePaths.at(idx);
	//return files.at(idx).second;
}


float ITKReader::getAlphaAt(int x, int y, int z) {
	//if (images != nullptr) {
		ImageType::IndexType index;
		index[0] = x;
		index[1] = y;
		index[2] = z;
		return images->GetPixel(index);
	//}
	//return 0.f;
}