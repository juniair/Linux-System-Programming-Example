#pragma pack(push, 1)
typedef struct {
	int ack_flag;
	int isExist;
	char message[32];
}Packet;
#pragma pack(pop)

#define True 1
#define False 0
#define ACK 1
#define NCK 0
