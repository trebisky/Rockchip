/* unpack.c -- 
 * Tom Trebisky  2-7-2022
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

/* This is sort of a check on rkdeveloptool and
 * replicates its "unpack" option.
 */

#define MAX_NAME_LEN            20

typedef enum {
        ENTRY_471       =1,
        ENTRY_472       =2,
        ENTRY_LOADER    =4,
} rk_entry_type;

#pragma pack(1)
typedef struct {
        uint16_t  year;
        uint8_t   month;
        uint8_t   day;
        uint8_t   hour;
        uint8_t   minute;
        uint8_t   second;
} rk_time;

#define  BOOT_RESERVED_SIZE 57
typedef struct {
        uint32_t        tag;
        uint16_t        size;
        uint32_t        version;
        uint32_t        mergerVersion;
        rk_time         releaseTime;
        uint32_t        chipType;
        uint8_t         code471Num;
        uint32_t        code471Offset;
        uint8_t         code471Size;
        uint8_t         code472Num;
        uint32_t        code472Offset;
        uint8_t         code472Size;
        uint8_t         loaderNum;
        uint32_t        loaderOffset;
        uint8_t         loaderSize;
        uint8_t         signFlag;
        uint8_t         rc4Flag;
        uint8_t         reserved[BOOT_RESERVED_SIZE];
} rk_boot_header;

typedef struct {
        uint8_t         size;
        rk_entry_type   type;
        uint16_t        name[MAX_NAME_LEN];
        uint32_t        dataOffset;
        uint32_t        dataSize;
        uint32_t        dataDelay;
} rk_boot_entry;
#pragma pack()

void
error ( char *msg )
{
	fprintf ( stderr, "%s\n", msg );
	exit ( 1 );
}

void
wide2str(const uint16_t* wide, char* str, int len)
{
        int i;

        for (i = 0; i < len; i++) {
                str[i] = (char) (wide[i] & 0xFF);
        }
}

void
rc4 (unsigned char *buf, int len )
{
        unsigned char S[256],K[256],temp;
        unsigned char key[16]={124,78,3,4,85,5,9,7,45,44,123,56,23,13,23,17};
	int i, j, t, x;

        j = 0;
        for(i=0; i<256; i++){
                S[i] = (unsigned char)i;
                j&=0x0f;
                K[i] = key[j];
                j++;
        }

        j = 0;
        for(i=0; i<256; i++){
                j = (j + S[i] + K[i]) % 256;
                temp = S[i];
                S[i] = S[j];
                S[j] = temp;
        }

        i = j = 0;
        for(x=0; x<len; x++){
                i = (i+1) % 256;
                j = (j + S[i]) % 256;
                temp = S[i];
                S[i] = S[j];
                S[j] = temp;
                t = (S[i] + (S[j] % 256)) % 256;
                buf[x] = buf[x] ^ S[t];
        }
}

char file_buf[128*1024];

void
writeEntry ( FILE *inFile, rk_boot_entry *ep )
{
	char name[MAX_NAME_LEN];
	int size;
	FILE *fout;

	fseek ( inFile, ep->dataOffset, SEEK_SET );
	size = ep->dataSize;
	if ( ! fread ( file_buf, size, 1, inFile ) )
	    error ( "failed reading entry" );

	printf ( "Write: %d bytes\n", size );

	rc4 ( file_buf, size );

	wide2str ( ep->name, name, MAX_NAME_LEN );
	fout = fopen ( name, "wb+" );
	fwrite ( file_buf, size, 1, fout );
	fclose ( fout );
}


#define MAX_ENTRY	8
int
main ( int argc, char **argv )
{
	FILE *inFile;
	rk_boot_header hdr;
	rk_boot_entry entrys[MAX_ENTRY];
	rk_boot_entry *ep;
	char name[MAX_NAME_LEN];
	int ne;
	int i;

	argc--;
	argv++;

	if ( argc < 1 )
	    error ( "usage: unpack file" );

	printf ( "Unpack: %s\n", argv[0] );

	inFile = fopen ( argv[0], "rb");
	if ( ! inFile )
	    error ( "Open failed" );

	if ( ! fread(&hdr, sizeof(rk_boot_header), 1, inFile) )
	    error ( "header read failed" );

	ne = hdr.code471Num + hdr.code472Num + hdr.loaderNum;
	printf ( "%d entrys\n", ne );

	if ( ! fread(entrys, sizeof(rk_boot_entry) * ne, 1, inFile) )
	    error ( "entry header read failed" );

	for ( i=0; i<ne; i++ ) {
	    ep = &entrys[i];
	    wide2str ( ep->name, name, MAX_NAME_LEN );
	    printf ( "%s, %d %d\n", name, ep->dataOffset, ep->dataSize );
	}

	writeEntry ( inFile, &entrys[0] );

	fclose ( inFile );
	return 0;
}

/* THE END */
