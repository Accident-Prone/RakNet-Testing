#include <stdio.h>
#include <RakPeerInterface.h>
#include <string.h>
#include <string>
#include <iostream>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <RakNetTypes.h>
#include <Kbhit.h>
#include <RakSleep.h>
#include <Gets.h>

using namespace std;

#define MAX_CLIENTS 10
#define SERVER_PORT 60000

enum GameMessages
{
	ID_GAME_MESSAGE_1=ID_USER_PACKET_ENUM+1
};

unsigned char GetPacketIdentifier(RakNet::Packet* p);

int main(void)
{
	char str[512], str1[512], serverIP[128];
	std::string name;
	RakNet::RakPeerInterface* peer = RakNet::RakPeerInterface::GetInstance();
	bool isServer;
	RakNet::Packet* packet;

	printf("(C)lient or (S)erver?\n");
	fgets(str, sizeof str, stdin);

	if ((str[0] == 'c') || (str[0] == 'C'))
	{
		RakNet::SocketDescriptor sd;
		peer->Startup(1, &sd, 1);
		isServer = false;
		printf("Client\n");
	}
	else
	{
		RakNet::SocketDescriptor sd(SERVER_PORT, 0);
		peer->Startup(MAX_CLIENTS, &sd, 1);
		isServer = true;
		printf("Server\n");
	}

	if (isServer)
	{
		printf("Starting the server.\n");
		// We need to let the server accept incoming connection from the clients
		peer->SetMaximumIncomingConnections(MAX_CLIENTS);
	}
	else
	{
		printf("Username: ");	
		cin >> name;
		name.append(": ");
		printf("Enter server IP or hit enter for 127.0.0.1 (this computer)\n");
		//fgets(serverIP, sizeof serverIP, stdin);
		cin >> serverIP;
		if (serverIP[0] == '\n')
		{
			strcpy(serverIP, "127.0.0.1");

		}
		printf("Starting the client...\n"); //Connection will commence after giving a message to send.\n");
		//printf("Message to send:\n");
		bool b = peer->Connect(serverIP, SERVER_PORT, 0, 0);
		if (b)
		{
			printf("failed...");
		}
		//fgets(str1, sizeof str1, stdin);
	}
	char* nameChar;
	nameChar = &name[0];
	char message[512];
	while (1)
	{
		if (isServer)
		{
			RakSleep(30);
			if (kbhit())
			{
				printf("Server: ");
				
				fgets(str1, sizeof str1, stdin);
				char message[sizeof(str1) + sizeof("Server: ")];
				strcpy(message, "Server: ");
				strcat(message, str1);
				peer->Send(message, (int)strlen(message) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			for (packet = peer->Receive();packet;peer->DeallocatePacket(packet), packet = peer->Receive())
			{
				switch (GetPacketIdentifier(packet))
				{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					printf("Another client has disconnected\n");
					break;
				case ID_REMOTE_CONNECTION_LOST:
					printf("Another client has lost the connection\n");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					printf("Another client has connected\n");
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					printf("Our connection request has been accepted\n");
					// Use a BitStream to write a custom message
					// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
					RakNet::BitStream bsOut;
					bsOut.Write((RakNet::MessageID)ID_GAME_MESSAGE_1);
					bsOut.Write("Hello world");
					peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->systemAddress, false);
					bsOut.Reset();
				}
				break;
				case ID_NEW_INCOMING_CONNECTION:
					printf("A connection is incoming\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full\n");
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					if (isServer)
					{
						printf("A client has disconnected\n");
					}
					else
					{
						printf("We have been disconnected\n");
					}
					break;
				case ID_CONNECTION_LOST:
					if (isServer)
					{
						printf("A client lost the connection\n");
					}
					else
					{
						printf("Connection lost\n");
					}
					break;
				case ID_GAME_MESSAGE_1:
				{
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					//bsIn.Reset();
				}
				break;
				default:
					//printf("Message with identifier %i has arrived\n", packet->data[0]);
					printf("%s", packet->data);

					sprintf(message, "%s", packet->data);
					peer->Send(message, (const int)strlen(message) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
					break;
				}
			}
		}
		else
		{
			Sleep(30);
			if (kbhit())
			{
				printf("Message to send:\n");
				fgets(str1, sizeof str1, stdin);
				char message[sizeof(str1) + sizeof(nameChar)];
				strcpy(message, nameChar);
				strcat(message, str1);
				peer->Send(message, (int)strlen(message) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
			for (packet = peer->Receive();packet;peer->DeallocatePacket(packet), packet = peer->Receive())
			{
				switch (GetPacketIdentifier(packet))
				{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					printf("Another client has disconnected\n");
					break;
				case ID_REMOTE_CONNECTION_LOST:
					printf("Another client has lost the connection\n");
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					printf("Another client has connected\n");
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					printf("Our connection request has been accepted\n");
					// Use a BitStream to write a custom message
					// Bitstreams are easier to use than sending casted structures, and handle endian swapping automatically
					printf("My ping to the server is %s:\n", packet->systemAddress.ToString(true));
				}
				break;
				case ID_NEW_INCOMING_CONNECTION:
					printf("A connection is incoming\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full\n");
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					if (isServer)
					{
						printf("A client has disconnected\n");
					}
					else
					{
						printf("We have been disconnected\n");
					}
					break;
				case ID_CONNECTION_LOST:
					if (isServer)
					{
						printf("A client lost the connection\n");
					}
					else
					{
						printf("Connection lost\n");
					}
					break;
				case ID_GAME_MESSAGE_1:
				{
					RakNet::RakString rs;
					RakNet::BitStream bsIn(packet->data, packet->length, false);
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
					bsIn.Read(rs);
					//bsIn.Reset();
				}
				default:
					printf("%s\n", packet->data);
					break;
				}
			}
		}
	}

	RakNet::RakPeerInterface::DestroyInstance(peer);

	return 0;

}
unsigned char GetPacketIdentifier(RakNet::Packet* p)
{
	if (p == 0)
		return 255;

	if ((unsigned char)p->data[0] == ID_TIMESTAMP)
	{
		RakAssert(p->length > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return (unsigned char)p->data[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
		return (unsigned char)p->data[0];
}