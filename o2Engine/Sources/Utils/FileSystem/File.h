#pragma once

#include <fstream>
#include "Utils/CommonTypes.h"
#include "Utils/String.h"

namespace o2
{
	enum class FileType { File, Image, Config, Atlas };

	// ----------
	// Input file
	// ----------
	class InFile
	{
		std::ifstream mIfstream; // Input stream
		String        mFilename; // File name
		bool          mOpened;   // True, if file was opened

	public:
		// Default constructor
		InFile();

		// Constructor with opening file
		InFile(const String& filename);

		// Destructor
		~InFile();

		// Opening file 
		bool Open(const String& filename);

		// Closing file
		bool Close();

		// Returns file data as string
		String ReadFullData();

		// Read full file data and return size of ridden data
		UInt ReadFullData(void *dataPtr);

		// Read data in dataPtr
		void ReadData(void *dataPtr, UInt bytes);

		// Sets caret position
		void SetCaretPos(UInt pos);

		// Return caret position
		UInt GetCaretPos();

		// Returns full data size
		UInt GetDataSize();

		// Returns true, if file was opened
		bool IsOpened() const;

		// Return file name
		const String& GetFilename() const;
	};

	// -----------
	// Output file
	// -----------
	class OutFile
	{
		std::ofstream mOfstream; // Output stream
		String        mFilename; // File name
		bool          mOpened;   // True, if file was opened

	public:
		// Default constructor
		OutFile();

		// Constructor with opening file
		OutFile(const String& filename);

		// Destcructor
		~OutFile();

		// Opening file 
		bool Open(const String& filename);

		// Close file
		bool Close();

		// Write some data from dataPtr
		void WriteData(const void* dataPtr, UInt bytes);

		// Returns true, if file was opened
		bool IsOpened() const;

		// Returns file name
		const String& GetFilename() const;
	};
}
