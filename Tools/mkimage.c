#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

struct head{
	uint8_t magic;
	uint8_t type;
	uint16_t depends;
	uint16_t ver;
	uint8_t time[6];
	uint32_t len;
	uint32_t addr;
	uint8_t crc[150];
	uint8_t reserve[30];
} head;

#define APP 0x01
#define BL 0x02

uint8_t gen_crc(uint8_t *str, uint32_t len)
{
	uint8_t crc = 0;
	uint32_t i;
	for(i = 0; i < len; i ++)
	{
		crc ^= str[i];
	}
	return crc;
}
uint8_t check_app_valid(char *name)
{
	FILE *fp;
	uint32_t len, i = 0;
	uint8_t buff[2048];
	fp = fopen(name, "rb");
	fread(&head, 1, sizeof(head), fp);
	while(len = fread(buff, 1, 2048, fp))
	{
		if(head.crc[i ++] != gen_crc(buff,len))
		{
			printf("upgrade package is invalid\r\n");
			return 0;
		}
	}
	fclose(fp);
	return 1;
}
int main(int argc, char *argv[])
{
	FILE *fp_src, *fp_dst;
	uint8_t *buf = NULL;
	head.magic = 'W';
	if(!strcasecmp(argv[1], "--application"))
	{
		head.type = APP;
		head.addr = 0x08010000;
	}
	else if(!strcasecmp(argv[1], "--bootloader"))
	{
		head.type = BL;
		head.addr = 0x08000000;
	}
	else 
	{
		check_app_valid(argv[2]);
		return 0;
	}
	head.depends = atoi(argv[2]);
	fp_src = fopen(argv[3], "rb");
	fp_dst = fopen(argv[4], "wb+");
	fseek(fp_src,0,SEEK_SET);
	fseek(fp_src,0,SEEK_END);
	head.len = ftell(fp_src);
	printf("%s\t%d\r\n", argv[3], head.len);
	if(head.len > 0)
	{
		uint32_t len;
		uint32_t ts, i;
		buf = malloc(head.len + sizeof(head));

		fseek(fp_src, 0, SEEK_SET);

		len = fread(&buf[sizeof(head)], 1, head.len, fp_src);
		printf("read len %d\r\n", len);
		for(i = 0; i < 200; i ++)
		{
			if(len >= 2048)
			{
				head.crc[i] = gen_crc(&buf[i * 2048 + sizeof(head)], 2048);
				len -= 2048;
			}
			else
			{
				head.crc[i] = gen_crc(&buf[i * 2048 + sizeof(head)], len);
				break;
			}
		}
		//time(&ts);
		//head.time = ts;
		memcpy(buf, &head, sizeof(head));
		len = 0;
		
		len = fwrite(buf,  1,sizeof(head) + head.len, fp_dst);
		printf("write len %d\r\n", len);
	}
	fclose(fp_src);
	fclose(fp_dst);
}
