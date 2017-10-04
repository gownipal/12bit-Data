/* Design Details
A circular buffer to store the last 32 read values so it easy to overwrite the buffer and to calculate offset of the buffer 
A regular buffer to store the sorted 32 values 
Both the buffers are 12 bit fields each 
*/
#include <stdio.h>
#include <stdbool.h>

#define MAX_ENTRIES 32
/*Optimize memory to save 12 bit values*/
typedef struct
{
    unsigned short value:12;
} bit;

static bit last_values[MAX_ENTRIES];
static bit sorted_values[MAX_ENTRIES];
static unsigned int head = 0;
static unsigned int num_read = 0;
/*
*Function Name: static void save_last_values(unsigned short value)
*This Function saves last 32 values read from file.
*Input: Value read from input file
*Return:  None
*/
static void save_last_values(unsigned short value)
{
	last_values[head].value = value;
	head++;
	/*Reset head to zero when max entries is reached for the circular buffer*/
	if (head >= MAX_ENTRIES) 
		head = 0;
}
/*
*Function Name: static void save_sorted_values(unsigned short value)
*This Function saves 32 largest values from file in sorted order.
*Input: Value read from input file
*Return:  None
*/
static void save_sorted_values(unsigned short value)
{
	int i = MAX_ENTRIES - 1;
	int j = 0;

	while (i >= 0)
	{
		if (value > sorted_values[i].value)
			break;
		i--;
	}

	if (i >= 0)
	{
		j = 0;
		while (j < i)
		{
			sorted_values[j].value = sorted_values[j + 1].value;
			j++;
		}
		sorted_values[i].value = value;
	}
}
/*
*Function Name: static unsigned short read_data(FILE *fp)
*This Function reads values from input file 
*Input: Pointer to input filename
*Return:  Value from the file
*/
static unsigned short read_data(FILE *fp)
{
	static unsigned short value = 0;
	unsigned char inputbyte = 0;
	size_t bytesread = 0;
    static unsigned char last_byte = 0;
	static bool flip = true;
	/* read one byte from our input file */
	bytesread = fread(&inputbyte, sizeof(inputbyte), 1, fp);
	if ((flip) && (bytesread > 0))
	{
		value = inputbyte << 4;
		/* Read last 4 bits */
		bytesread = fread(&(inputbyte), sizeof(inputbyte), 1, fp);
		if (bytesread > 0)
		{
			value |= (((inputbyte) & (0xF0)) >> 4);
			last_byte = inputbyte;
			flip = false;
		} else
		{
			value = 0xFFF1;
		}
	} else if (bytesread > 0)
	{
		value = (last_byte & (0x0F)) << 8;
		value |= (0x00FF & inputbyte);
		flip = true;
	} else
	{
		value =  0xFFF2;
	}

	return value;
}
/*
*Function Name: static void write_data(const char *outfile) 
*This Function writes the sorted values,last set of values to output file provided. 
*Input: Output Filename String 
*Return: None 
*/
static void write_data(const char *outfile)
{
	int value = 0;
	int i = 0,j=0,k=0;
	FILE *fp = NULL;

	/* Open output file */
	fp = fopen(outfile, "w");
	if (fp != NULL)
	{
		/* print the max 32 first */
		fprintf(fp, "--Sorted Max %d Values--\r\n",MAX_ENTRIES);
		for (i = 0; i < MAX_ENTRIES; i++)
		{

			if (i < num_read)
			{
				if (num_read < MAX_ENTRIES)
				{
					k = i + MAX_ENTRIES - num_read;	
					value = sorted_values[k].value;
				}
				else
					value = sorted_values[i].value;
			}
			else
				break;
			if (value >= 0)
				fprintf(fp, "%i\r\n", value);
		}

		/* print the last 32 next */
		fprintf(fp, "--Last %d Values--\r\n", MAX_ENTRIES);
		for (i = 0; i < MAX_ENTRIES; i++)
		{
			if (num_read >= MAX_ENTRIES)
			{
				j = (head + i)% MAX_ENTRIES;
				value = last_values[j].value;
			}
			else if(i<num_read) {
				value = last_values[i].value;
			}else
				break;
			
			
			fprintf(fp, "%i\r\n", value);

		}

		/* done, close the file */
		fclose(fp);
	} else
	{
		printf("Error opening %s\n", outfile);
	}

	return;
}

int main(int argc, char *argv[])
{
	char *inpfile;
	char *outfile;
	inpfile = argv[1];
	outfile = argv[2];
	FILE *fp;
	unsigned short value;
	/* Open the input file*/
	fp = fopen(inpfile, "rb");
	if (!fp)
	{
		printf("Unable to open file");
		return 1;
	}
	/* Read the values from file*/
	value = read_data(fp);
	while (!(value & 0xF000))
	{
		num_read++;
		save_last_values(value);
		save_sorted_values(value);
		value = read_data(fp);
	}
	fclose(fp);
	fp = NULL;
	/*Write the values to Output file*/
	write_data(outfile);

	return 0;
}
