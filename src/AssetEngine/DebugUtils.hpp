#ifndef DEBUGUTILS_H__
#define DEBUGUTILS_H__

#include "pch.h"

#if defined (_DEBUG)
	#define DEBUG_UTILS_ENABLED 1
#else
	#define DEBUG_UTILS_ENABLED 0
#endif

namespace DebugUtils
{
#if DEBUG_UTILS_ENABLED

	//PRINT AS TEXT
	inline void PrintAsText(const Blob& blob, bool addNewline = true)
	{
		std::cout.write((const char*)blob.GetData(), blob.GetSize());
		if (addNewline) std::cout << "\n";
	}
	inline void PrintAsText(const UINT8* data, UINT64 size, bool addNewline = true)
	{
		std::cout.write((const char*)data, size);
		if (addNewline) std::cout << "\n";
	}
	inline void PrintAsText(const Memory& memory, bool addNewline = true)
	{
		PrintAsText(*memory.GetBlob(), addNewline);
	}

	//PRINT AS HEX
	inline void PrintAsHex(const UINT8* data, UINT64 size, UINT64 bytesPerLine = 16)
	{
		for (UINT64 i = 0; i < size; ++i)
		{
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)data[i] << " ";

			if ((i + 1) % bytesPerLine == 0) std::cout << "\n";
		}
		if (size % bytesPerLine != 0) std::cout << "\n";
		std::cout << std::dec;
	}
	inline void PrintAsHex(const Blob& blob, UINT64 bytesPerLine = 16)
	{
		PrintAsHex(blob.GetData(), blob.GetSize(), bytesPerLine);
	}
	inline void PrintAsHex(const Memory& memory, UINT64 bytesPerLine = 16)
	{
		PrintAsHex(*memory.GetBlob(), bytesPerLine);
	}

	//HEXDUMP
	inline void HexDump(const UINT8* data, UINT64 size, UINT64 maxBytes = 256)
	{
		UINT64 bytesToShow = (size < maxBytes) ? size : maxBytes;

		for (UINT64 i = 0; i < bytesToShow; i += 16)
		{
			std::cout << std::hex << std::setw(8) << std::setfill('0') << i << ": ";

			// HEX
			for (UINT64 j = 0; j < 16; j++)
			{
				if (i + j < bytesToShow)
				{
					std::cout << std::setw(2) << std::setfill('0') << (int)data[i + j] << " ";
				}
				else
				{
					std::cout << "   ";
				}
			}

			std::cout << " | ";

			// ASCII
			for (UINT64 j = 0; j < 16 && (i + j) < bytesToShow; j++)
			{
				char c = data[i + j];
				std::cout << (isprint(c) ? c : '.');
			}

			std::cout << "\n";
		}

		if (size > maxBytes)
		{
			std::cout << "... (" << (size - maxBytes) << " more bytes)\n";
		}

		std::cout << std::dec;
	}
	inline void HexDump(const Blob& blob, UINT64 maxBytes = 256)
	{
		HexDump(blob.GetData(), blob.GetSize(), maxBytes);
	}
	inline void HexDump(const Memory& memory, UINT64 maxBytes = 256)
	{
		HexDump(*memory.GetBlob(), maxBytes);
	}

	//COMPARE
	inline bool CompareData(const UINT8* data1, const UINT8* data2, UINT64 size)
	{
		return memcmp(data1, data2, size) == 0;
	}
	inline bool CompareBlobs(const Blob& blob1, const Blob& blob2)
	{
		if (blob1.GetSize() != blob2.GetSize())
			return false;
		return CompareData(blob1.GetData(), blob2.GetData(), blob1.GetSize());
	}

	//DEBUG PRINT
	inline void PrintSuccess(const std::string& msg)
	{
		std::cout << "[SUCCESS] " << msg << "\n";
	}
	inline void PrintError(const std::string& msg)
	{
		std::cout << "[ERROR] " << msg << "\n";
	}
	inline void PrintInfo(const std::string& msg)
	{
		std::cout << "[INFO] " << msg << "\n";
	}
	inline void PrintTitle(const std::string& title)
	{
		std::cout << "\n=== " << title << " ===\n";
	}
	inline void PrintSection(const std::string& title)
	{
		std::cout << "|==========================================|\n";
		std::cout << "| " << title << "\n";
		std::cout << "|==========================================|\n";
	}

	//DATA STATS
	struct DataStats
	{
		UINT64 size;
		UINT8 minValue;
		UINT8 maxValue;
		double average;
		UINT64 zeros;
		UINT64 printable;
	};
	inline DataStats AnalyzeData(const UINT8* data, UINT64 size)
	{
		DataStats stats = { 0 };
		if (size == 0) return stats;

		stats.size = size;
		stats.minValue = 255;
		stats.maxValue = 0;
		UINT64 sum = 0;

		for (UINT64 i = 0; i < size; i++)
		{
			UINT8 byte = data[i];

			if (byte < stats.minValue) stats.minValue = byte;
			if (byte > stats.maxValue) stats.maxValue = byte;

			sum += byte;

			if (byte == 0) stats.zeros++;
			if (isprint(byte)) stats.printable++;
		}

		stats.average = (double)sum / size;

		return stats;
	}
	inline void PrintStats(const DataStats& stats)
	{
		std::cout << "Data Statistics:\n";
		std::cout << "  Size:      " << stats.size << " bytes\n";
		std::cout << "  Min:       " << (int)stats.minValue << "\n";
		std::cout << "  Max:       " << (int)stats.maxValue << "\n";
		std::cout << "  Average:   " << std::fixed << std::setprecision(2) << stats.average << "\n";
		std::cout << "  Zeros:     " << stats.zeros << " ("
			<< (stats.size > 0 ? (stats.zeros * 100 / stats.size) : 0) << "%)\n";
		std::cout << "  Printable: " << stats.printable << " ("
			<< (stats.size > 0 ? (stats.printable * 100 / stats.size) : 0) << "%)\n";
	}

	//DATA TESTS
	inline void GenerateRandomData(UINT8* buffer, UINT64 size)
	{
		for (UINT64 i = 0; i < size; i++)
		{
			buffer[i] = rand() % 256;
		}
	}
	inline void GeneratePatternData(UINT8* buffer, UINT64 size, UINT8 pattern = 0xAB)
	{
		for (UINT64 i = 0; i < size; i++)
		{
			buffer[i] = (UINT8)(pattern + (i % 16));
		}
	}
	struct FakeImageData
	{
		UINT32 width;
		UINT32 height;
		UINT32 bpp;  //bitsPerPixel
	};
	inline Blob* CreateFakeImageBlob(UINT32 width, UINT32 height)
	{
		UINT32 pixelCount = width * height;
		UINT32 totalSize = sizeof(FakeImageData) + pixelCount * 4;  //RGBA

		Blob* blob = new Blob();
		blob->Reserve(totalSize);

		// Header
		FakeImageData header;
		header.width = width;
		header.height = height;
		header.bpp = 32;
		blob->Append((UINT8*)&header, sizeof(FakeImageData));

		// Pixels (gradient)
		for (UINT32 i = 0; i < pixelCount; i++)
		{
			UINT8 pixel[4] = {
				(UINT8)(i % 256),           // R
				(UINT8)((i / 256) % 256),   // G
				(UINT8)((i / 512) % 256),   // B
				255                         // A
			};
			blob->Append(pixel, 4);
		}

		return blob;
	}
#endif

}

#endif // !DEBUGUTILS_H__
