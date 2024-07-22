#include "my_header.h"

uInt convert(uInt test)
{
	uint8_t data[4];
	memcpy(&data, &test, sizeof(data));
	return ((uint32_t)data[3] << 0) | ((uint32_t)data[2] << 8) | ((uint32_t)data[1] << 16) | ((uint32_t)data[0] << 24);
}

void filterSub(uChar *data, size_t line, const uInt byteLen, uInt width)
{
	for (int i = 0; i < byteLen; i++)
		for (uInt j = 1; j < width; j++)
			data[i + 1 + j * byteLen + line * (width * byteLen + 1)] +=
				data[i + 1 + (j - 1) * byteLen + line * (width * byteLen + 1)];
}

void filterUp(uChar *data, size_t line, const uInt byteLen, uInt width)
{
	if (line == 0)
		return;
	for (int i = 0; i < byteLen; i++)
		for (uInt j = 0; j < width; j++)
			data[i + 1 + j * byteLen + line * (width * byteLen + 1)] +=
				data[i + 1 + j * byteLen + (line - 1) * (width * byteLen + 1)];
}

void filterAverage(uChar *data, size_t line, const uInt byteLen, uInt width)
{
	for (int i = 0; i < byteLen; i++)
		for (uInt j = 0; j < width; j++)
		{
			uChar predLeft = j == 0 ? 0 : data[i + 1 + (j - 1) * byteLen + line * (width * byteLen + 1)];
			uChar predUp = line == 0 ? 0 : data[i + 1 + j * byteLen + (line - 1) * (width * byteLen + 1)];
			data[i + 1 + j * byteLen + line * (width * byteLen + 1)] += (predLeft + predUp) / 2;
		}
}

void filterPaeth(uChar *data, size_t line, const uInt byteLen, uInt width)
{
	for (int i = 0; i < byteLen; i++)
		for (uInt j = 0; j < width; j++)
		{
			uChar this;
			int predLeft = j == 0 ? 0 : data[i + 1 + (j - 1) * byteLen + line * (width * byteLen + 1)];
			int predUp = line == 0 ? 0 : data[i + 1 + j * byteLen + (line - 1) * (width * byteLen + 1)];
			int predLeftUp = (j == 0 || line == 0) ? 0 : data[i + 1 + (j - 1) * byteLen + (line - 1) * (width * byteLen + 1)];
			int pa = abs(predUp - predLeftUp);
			int pb = abs(predLeft - predLeftUp);
			int pc = abs(predLeft + predUp - 2 * predLeftUp);
			if (pa <= pb && pa <= pc)
				this = predLeft;
			else if (pb <= pc)
				this = predUp;
			else
				this = predLeftUp;
			data[i + 1 + j * byteLen + line * (width * byteLen + 1)] += this;
		}
}

int exitEmpty(int errorCode, char message[])
{
	if (message)
		fprintf(stderr, "%s", message);
	return errorCode;
}

int exitFile(int errorCode, char message[], FILE *fp)
{
	if (fp)
		fclose(fp);
	return exitEmpty(errorCode, message);
}

int exitCharIntFile(int errorCode, char message[], uChar *a, uInt *b, FILE *fp)
{
	if (a)
		free(a);
	if (b)
		free(b);
	return exitFile(errorCode, message, fp);
}

int exitCharIntPlteFile(int errorCode, char message[], uChar *a, uInt *b, uChar *plte, FILE *fp)
{
	if (plte)
		free(plte);
	return exitCharIntFile(errorCode, message, a, b, fp);
}

int exitCharChar(int errorCode, char message[], uChar *a, uChar *b)
{
	if (a)
		free(a);
	if (b)
		free(b);
	return exitEmpty(errorCode, message);
}

int exitCharCharFile(int errorCode, char message[], uChar *a, uChar *b, FILE *fp)
{
	if (fp)
		fclose(fp);
	return exitCharChar(errorCode, message, a, b);
}

int checkRGB(uChar typeColor, const uChar *plteData, uInt plteLen)
{
	if (typeColor != 3)
		return typeColor == 2;
	for (uInt k = 0; k < plteLen / 3; k++)
	{
		if (!(plteData[3 * k] == plteData[3 * k + 1] == plteData[3 * k + 2]))
		{
			return 1;
		}
	}
	return 0;
}

int writer(uInt isRGbPLTE, char *fileName, uInt width, uInt high, uInt byteLen, uChar *newBuf, uChar *plteData, uChar colorType)
{
	FILE *wr;
	if (!(wr = fopen(fileName, "wb")))
	{
		return exitCharChar(ERROR_CANNOT_OPEN_FILE, "File didn't open", newBuf, plteData);
	}
	else if (fprintf(wr, "P%d\n%u %u\n255\n", isRGbPLTE ? 6 : 5, width, high) < 0)
	{
		return exitCharCharFile(ERROR_UNKNOWN, "Invalid writing", newBuf, plteData, wr);
	}
	for (size_t j = 0; j < high; j++)
	{
		switch (newBuf[j * width * byteLen + j])
		{
		case 1:
			filterSub(newBuf, j, byteLen, width);
			break;
		case 2:
			filterUp(newBuf, j, byteLen, width);
			break;
		case 3:
			filterAverage(newBuf, j, byteLen, width);
			break;
		case 4:
			filterPaeth(newBuf, j, byteLen, width);
			break;
		case 0:
			break;
		default:
			return exitCharCharFile(ERROR_UNKNOWN, "Unknown filter", newBuf, plteData, wr);
		}
		if (colorType != 3)
		{
			if (fwrite(newBuf + 1 + j * (width * byteLen + 1), SuChar, width * byteLen, wr) != width * byteLen)
			{
				return exitCharCharFile(ERROR_UNKNOWN, "Invalid writing", newBuf, plteData, wr);
			}
		}
		else
		{
			int log = isRGbPLTE ? 3 : 1;
			for (size_t k = 0; k < width; k++)
			{
				if (fwrite((plteData + 3 * newBuf[1 + j * (width * byteLen + 1) + k]), SuChar, log, wr) != log)
				{
					return exitCharCharFile(ERROR_UNKNOWN, "Invalid writing", newBuf, plteData, wr);
				}
			}
		}
	}
	return exitCharCharFile(SUCCESS, NULL, newBuf, plteData, wr);
}

int main(int args, char *argv[])
{
	if (args != 3)
	{
		return exitEmpty(ERROR_PARAMETER_INVALID, "Invalid count of arguments");
	}

	FILE *fp;
	if (!(fp = fopen(argv[1], "rb")))
	{
		return exitEmpty(ERROR_CANNOT_OPEN_FILE, "File didn't open");
	}
	uChar arrPNGSignature[8], name[4], other[5];

	if (fread(arrPNGSignature, SuChar, 8, fp) != 8)
	{
		return exitFile(ERROR_DATA_INVALID, "Invalid bytes of signature", fp);
	}
	const uChar pngValid[] = { 137, 'P', 'N', 'G', '\r', '\n', '\x1A', '\n' }, checkNameIHDR[] = "IHDR",
				checkNamePLTE[] = "PLTE", checkNameIDAT[] = "IDAT", checkNameIEND[] = "IEND";

	if (memcmp(arrPNGSignature, pngValid, 8) != 0)
	{
		return exitFile(ERROR_DATA_INVALID, "File isn't PNG", fp);
	}

	uInt mem, width, high, control;
	if (fread(&mem, SuInt, 1, fp) != 1 || fread(name, SuChar, 4, fp) != 4 || fread(&width, SuInt, 1, fp) != 1 ||
		fread(&high, SuInt, 1, fp) != 1 || fread(other, SuChar, 5, fp) != 5 || fread(&control, SuInt, 1, fp) != 1)
	{
		return exitFile(ERROR_DATA_INVALID, "Invalid bytes", fp);
	}
	width = convert(width);
	high = convert(high);
	control = convert(control);

	if ((mem = convert(mem)) != 13 || memcmp(name, checkNameIHDR, 4) != 0 || (other[1] != 0 && other[1] != 2 && other[1] != 3) ||
		other[0] != 8 || other[2] != 0 || other[3] != 0 || (other[4] != 0 && other[4] != 1))
	{
		return exitFile(ERROR_DATA_INVALID, "Invalid data", fp);
	}

	uInt *sizeControl, dataLen = 0, plteLen = 0, valid[] = { 0, other[1] != 3, 0 };
	uChar chunkName[4], *data = 0, *data2, *plteData = 0;
	size_t move = 0;

	if (!(sizeControl = (uInt *)malloc(SuInt * 2)))
	{
		return exitFile(ERROR_OUT_OF_MEMORY, "Out of memory", fp);
	}

	while (1)
	{
		if (fread(sizeControl, SuInt, 1, fp) != 1 || fread(chunkName, SuChar, 4, fp) != 4)
		{
			return exitCharIntFile(ERROR_DATA_INVALID, "Invalid bytes", data, sizeControl, fp);
		}
		sizeControl[0] = convert(sizeControl[0]);

		if (memcmp(checkNameIDAT, chunkName, 4) == 0)
		{
			if (!valid[0] && move == 0)
				valid[0] = 1;
			else if (other[3] == 3 && move == 0)
			{
				return exitCharIntFile(ERROR_DATA_INVALID, "IDAT chunk before PLTE chunk", data, sizeControl, fp);
			}
			dataLen += sizeControl[0] * SuChar;

			if (!(data2 = (uChar *)realloc(data, dataLen)))
			{
				return exitCharIntFile(ERROR_OUT_OF_MEMORY, "Out of memory", data2, sizeControl, fp);
			}
			else if (fread(data2 + move, SuChar * sizeControl[0], 1, fp) != 1 || fread(sizeControl + 1, SuInt, 1, fp) != 1)
			{
				return exitCharIntFile(ERROR_DATA_INVALID, "Invalid bytes", data2, sizeControl, fp);
			}
			data = data2;
			move += SuChar * sizeControl[0];
		}
		else if (memcmp(chunkName, checkNamePLTE, 4) == 0)
		{
			if (other[1] == 0 || move != 0 || sizeControl[0] % 3 != 0)
			{
				return exitCharIntFile(ERROR_DATA_INVALID, "Invalid bytes", data, sizeControl, fp);
			}
			else if (!(plteData = (uChar *)malloc(SuChar * sizeControl[0])))
			{
				return exitCharIntFile(ERROR_OUT_OF_MEMORY, "Out of memory", data, sizeControl, fp);
			}
			else if (fread(plteData, SuChar * sizeControl[0], 1, fp) != 1 || fread(sizeControl + 1, SuInt, 1, fp) != 1)
			{
				return exitCharIntPlteFile(ERROR_DATA_INVALID, "Invalid bytes", data, sizeControl, plteData, fp);
			}
			plteLen = sizeControl[0];
			valid[1] = 1;
			if (other[1] != 3)
			{
				free(plteData);
				plteData = NULL;
			}
		}
		else if (memcmp(checkNameIEND, chunkName, 4) == 0)
		{
			valid[2] = 1;
			if (fread(sizeControl + 1, SuInt, 1, fp) != 1)
			{
				if (plteData)
					free(plteData);
				return exitCharIntFile(ERROR_DATA_INVALID, "Invalid bytes", data, sizeControl, fp);
			}
			else if (!feof(fp))
				break;
			return exitCharIntPlteFile(ERROR_UNKNOWN, "Invalid chunk", data, sizeControl, plteData, fp);
		}
		else
		{
			uChar *extraMemory;
			if (!(extraMemory = (uChar *)malloc(SuChar * sizeControl[0])))
			{
				return exitCharIntPlteFile(ERROR_OUT_OF_MEMORY, "Out of memory", data, sizeControl, plteData, fp);
			}
			else if (fread(extraMemory, SuChar * sizeControl[0], 1, fp) != 1 || fread(sizeControl + 1, SuInt, 1, fp) != 1)
			{
				free(extraMemory);
				return exitCharIntPlteFile(ERROR_DATA_INVALID, "Invalid data", data, sizeControl, plteData, fp);
			}
			free(extraMemory);
		}

		sizeControl[1] = convert(sizeControl[1]);
	}
	free(sizeControl);
	fclose(fp);

	if (!(valid[0] && valid[1] && valid[2]) || (data == 0 && other[1] == 3))
	{
		return exitCharChar(ERROR_UNKNOWN, "Invalid chunks", data, plteData);
	}

	const uInt byteLen = other[1] == 2 ? 3 : 1;
	uChar *newBuf;

#if defined(ZLIB)
	uLongf help = high * (width * byteLen + 1);
	uLongf *newLen = &help;
	if (!(newBuf = (uChar *)malloc(*newLen)))
	{
		return exitCharChar(ERROR_OUT_OF_MEMORY, "Out of memory", data, plteData);
	}

	if (uncompress(newBuf, newLen, data, dataLen) != Z_OK)
	{
		free(plteData);
		return exitCharChar(ERROR_UNSUPPORTED, "Error decompression", data, newBuf);
	}
#elif defined(LIBDEFLATE)
	struct libdeflate_decompressor *decomp;
	size_t libLen = high * (width * byteLen + 1);
	if (!(newBuf = (uChar *)malloc(libLen)))
	{
		return exitCharChar(ERROR_OUT_OF_MEMORY, "Out of memory", data, plteData);
	}

	if (!(decomp = libdeflate_alloc_decompressor()) || libdeflate_zlib_decompress(decomp, data, dataLen, newBuf, libLen, NULL) != 0)
	{
		libdeflate_free_decompressor(decomp);
		free(plteData);
		return exitCharChar(ERROR_UNSUPPORTED, "Error decompression", data, newBuf);
	}
	libdeflate_free_decompressor(decomp);
#elif defined(ISAL)
	struct inflate_state infStat;
	isal_inflate_init(&infStat);
	size_t isalLen = high * (width * byteLen + 1);
	if (!(newBuf = (uChar *)malloc(isalLen)))
	{
		return exitCharChar(ERROR_OUT_OF_MEMORY, "Out of memory", data, plteData);
	}

	infStat.next_in = data;
	infStat.avail_in = dataLen;
	infStat.next_out = newBuf;
	infStat.avail_out = isalLen;

	struct isal_zlib_header isHead;

	if (isal_read_zlib_header(&infStat, &isHead) != ISAL_DECOMP_OK || isal_inflate(&infStat) != ISAL_DECOMP_OK)
	{
		free(plteData);
		return exitCharChar(ERROR_UNSUPPORTED, "Error decompression", data, newBuf);
	}
#else
#error Invalid macros
#endif
	free(data);

	return writer(checkRGB(other[1], plteData, plteLen), argv[2], width, high, byteLen, newBuf, plteData, other[1]);
}