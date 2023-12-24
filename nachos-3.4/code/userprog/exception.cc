// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#define MAX_STRING_LENGTH 2056
#define MAX_FILE_LENGTH 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

// Input: - User space address (int)
// - Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space

char *User2System(int virtAddr, int limit);
int System2User(int virtAddr, int len, char *buffer);
void IncreasePC();

void ReadIntHandler();
void PrintIntHandler();
void ReadCharHandler();
void PrintCharHandler();
void ReadStringHandler();
void PrintStringHandler();
void ReadIntHandler();

void CreateFileHandler();
void OpenHandler();
void CloseHandler();
void ReadHandler() ;
void WriteHandler();
void SeekHandler();
void ExecHandler();
void JoinHandler();
void ExitHandler();

void ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	switch (which)
	{
	case NoException:
		return;
	case PageFaultException:
		DEBUG('a', "\nNo valid translation found\n");
		printf("\nNo valid translation found\n");
		interrupt->Halt();
		break;
	case ReadOnlyException:
		DEBUG('a', "\nWrite attempted to page marked read-only\n");
		printf("\nWrite attempted to page marked read-only\n");
		interrupt->Halt();
		break;
	case BusErrorException:
		DEBUG('a', "\nTranslation resulted invalid physical address\n");
		printf("\nTranslation resulted invalid physical address\n");
		interrupt->Halt();
		break;
	case AddressErrorException:
		DEBUG('a', "\nUnaligned reference or one that was beyond the end of the address space\n");
		printf("\nUnaligned reference or one that was beyond the end of the address space\n");
		interrupt->Halt();
		break;
	case OverflowException:
		DEBUG('a', "\nInteger overflow\n");
		printf("\nInteger overflow\n");
		interrupt->Halt();
		break;
	case IllegalInstrException:
		DEBUG('a', "\nUnimplemented or reserved instr\n");
		printf("\nUnimplemented or reserved instr\n");
		interrupt->Halt();
		break;
	case NumExceptionTypes:
		DEBUG('a', "\nNumber exception types\n");
		printf("\nNumber exception types\n");
		interrupt->Halt();
		break;
	case SyscallException:
	{
		switch (type)
		{
		case SC_Halt:
		{
			DEBUG('a', "\nShutdown, initiated by user program.\n");
			printf("\nShutdown, initiated by user program\n");
			interrupt->Halt();
			break;
		}
		case SC_ReadInt:
			ReadIntHandler();
			break;
		case SC_PrintInt:
			PrintIntHandler();
			break;
		case SC_ReadChar:
			ReadCharHandler();
			break;
		case SC_PrintChar:
			PrintCharHandler();
			break;
		case SC_ReadString:
			ReadStringHandler();
			break;
		case SC_PrintString:
			PrintStringHandler();
			break;
		case SC_Create:
			CreateFileHandler();
			break;
		case SC_Open : 
			OpenHandler();
			break;
		case SC_Close :
			CloseHandler();
			break;
		case SC_Read :
			ReadHandler();
			break;
		case SC_Write : 
			WriteHandler();
			break;
		case SC_Seek :
			SeekHandler();
			break;
		case SC_Exec :
			ExecHandler();
			break;
		case SC_Join:
			JoinHandler();
			break;
		case SC_Exit :
			ExitHandler();
			break;
		default:
			IncreasePC(); //and do nothing
			break;
		}
			break;
	}
	default:
		printf("\nUnexpected user mode exception %d\n", which);
		ASSERT(FALSE);
		break;
	}
}

void ReadIntHandler()
{
	//Chuoi luu so dang char
	char *numberchar;
	//Do dai toi da cua chuoi
	int MAX_BUFFER = 256;
	numberchar = new char[MAX_BUFFER];
	//Do dai duoc doc
	int length = gSynchConsole->Read(numberchar, MAX_BUFFER);
	//Khoi tao so nguyen bang 0
	int number = 0;
	//Mac dinh la so duong
	bool isNegative = false;
	//Vi tri bat dau cua so, mac dinh la 0
	int head = 0;
	//Vi tri ket thuc cua so, mac dinh la chieu dai duoc doc
	int tail = length - 1;

	//Kiem tra xem co phai so am khong
	if (numberchar[0] == '-')
	{
		isNegative = true;
		//Doi vi tri bat dau so sang 1 ky tu
		head = 1;
	}

	//Kiem tra so co hop le khong
	for (int i = head; i < length; i++)
	{
		//Kiem tra chuoi co xuat hien ky tu khac ky tu so
		if (numberchar[i] < '0' || numberchar[i] > '9')
		{
			printf("Khong phai so nguyen, tra ve so 0 mac dinh\n");
			DEBUG('a', "Khong phai so nguyen, tra ve so 0 mac dinh\n");
			//Tra ve thanh ghi r2 so 0 mac dinh
			machine->WriteRegister(2, 0);
			IncreasePC();
			delete numberchar;
			return;
		}
	}

	//Chuyen so tu char sang int
	for (int i = head; i <= tail; i++)
	{
		number = number * 10 + (int)(numberchar[i] - 48);
	}

	//Chuyen thanh so am neu co
	if (isNegative)
	{
		number = number * -1;
	}
	//Tra ve thanh ghi r2
	machine->WriteRegister(2, number);
	IncreasePC();
	delete numberchar;
	return;
}

void PrintIntHandler()
{
	//Doc so tu thanh ghi r4
	int number = machine->ReadRegister(4);
	if (number == 0)
	{
		gSynchConsole->Write("0", 1); // In ra man hinh so 0
		IncreasePC();
		return;
	}
	//Mac dinh la so duong
	bool isNegative = false;
	//Do dai so
	int length = 0;
	int head = 0;
	//Kiem tra so co am khong
	if (number < 0)
	{
		isNegative = true;
		//Chuyen thanh so duong cho de thao tac
		number = number * -1;
		//Nhuong 1 vi tri cho dau tru
		head = 1;
	}

	//Bien tam de dem do dai so
	int temp = number;
	//Dem do dai
	while (temp > 0)
	{
		length = length + 1;
		temp = temp / 10;
	}

	//Chuoi so de in ra man hinh
	char *numberchar;
	//Chuoi so nguoc
	char *reverse;
	//Do dai toi da cua so
	int MAX_BUFFER = 256;
	numberchar = new char[MAX_BUFFER];
	reverse = new char[MAX_BUFFER];

	//Chuyen so thanh chuoi nguoc
	for (int i = 0; i < length; i++)
	{
		reverse[i] = (char)((number % 10) + 48);
		number = number / 10;
	}
	if (isNegative)
	{
		//Them dau tru
		numberchar[0] = '-';
		//Dao lai chuoi
		for (int i = 0; i < length; i++)
		{
			numberchar[length - i] = reverse[i];
		}
		//In so ra man hinh
		gSynchConsole->Write(numberchar, length + 1);
		delete numberchar;
		IncreasePC();
		return;
	}

	//Dao lai chuoi
	for (int i = 0; i < length; i++)
	{
		numberchar[length - i - 1] = reverse[i];
	}
	//In so ra man hinh
	gSynchConsole->Write(numberchar, length);
	delete numberchar;
	IncreasePC();
	return;
}

void ReadCharHandler()
{
	// Tao buffer de doc size 255
	char *buffer = new char[255];
	int len = gSynchConsole->Read(buffer, 255);

	// Xu ly loi khi doc
	if (len == -1)
	{
		printf("Doc ki tu bi loi");
		DEBUG('a', "\nERROR: reading char.");
		return;
	}

	// Ghi ki tu vua doc vao thanh ghi r2
	char c = buffer[0];
	machine->WriteRegister(2, c);

	delete buffer; // giai phong vung cap phat
	IncreasePC();
	return;
}

void PrintCharHandler()
{
	// Doc ki tu o thanh ghi r4
	char c = (char)machine->ReadRegister(4);
	// In ra console
	gSynchConsole->Write(&c, 1);
	IncreasePC();
	return;
}

void ReadStringHandler()
{
	int bufferAddr, maxStringLengthInput;
	char *inputBuffer;

	bufferAddr = machine->ReadRegister(4);			 // Get the address of the 'buffer' parameter from register 4
	maxStringLengthInput = machine->ReadRegister(5); // Get the maximum length of the input string from register 5

	if (maxStringLengthInput > MAX_STRING_LENGTH)
	{
		DEBUG('a', "\n Not enough buffer to read tring");
		printf("\n Not enough buffer to read tring");
		IncreasePC();
		return;
	}

	inputBuffer = User2System(bufferAddr, maxStringLengthInput); // Copy the string from User Space to System Space
	gSynchConsole->Read(inputBuffer, maxStringLengthInput);		 // Use SynchConsole's Read function to read the string
	System2User(bufferAddr, maxStringLengthInput, inputBuffer);	 // Copy the string from System Space to User Space
	delete[] inputBuffer;										 // Free the allocated memory for the buffer
	IncreasePC();												 // Increment Program Counter
	return;
}

void PrintStringHandler()
{
	int bufferAddr;
	char *inputBuffer;

	bufferAddr = machine->ReadRegister(4);					  // Get the address of the 'buffer' parameter from register 4
	inputBuffer = User2System(bufferAddr, MAX_STRING_LENGTH); // Copy the string from User Space to System Space with a buffer of 255 characters

	int length = 0;

	while (inputBuffer[length] != 0)
		length++; // Calculate the actual length of the string

	if (length > MAX_STRING_LENGTH)
	{
		DEBUG('a', "\n Not enough buffer to printString");
		printf("\n Not enough buffer to printString");
		IncreasePC();
		return;
	}

	gSynchConsole->Write(inputBuffer, length + 1); // Use SynchConsole's Write function to print the string
	delete[] inputBuffer;						   // Free the allocated memory for the buffer
	IncreasePC();								   // Tang Program Counter
	return;
}

void CreateFileHandler(){
	// Input: Dia chi tu vung nho user cua ten file
			// Output: -1 = Loi, 0 = Thanh cong
			// Chuc nang: Tao ra file voi tham so la ten file
			int virtAddr;
			char* filename;
			DEBUG('a', "\n SC_CreateFile call ...");
			DEBUG('a', "\n Reading virtual address of filename");

			virtAddr = machine->ReadRegister(4); //Doc dia chi cua file tu thanh ghi R4
			DEBUG('a', "\n Reading filename.");
			
			//Sao chep khong gian bo nho User sang System, voi do dang toi da la (32 + 1) bytes
			filename = User2System(virtAddr, MAX_FILE_LENGTH + 1);
			if (strlen(filename) == 0)
			{
				printf("\n File name is not valid");
				DEBUG('a', "\n File name is not valid");
				machine->WriteRegister(2, -1); //Return -1 vao thanh ghi R2
				//IncreasePC();
				//return;
				return;
			}
			
			if (filename == NULL)  //Neu khong doc duoc
			{
				printf("\n Not enough memory in system");
				DEBUG('a', "\n Not enough memory in system");
				machine->WriteRegister(2, -1); //Return -1 vao thanh ghi R2
				delete filename;
				//IncreasePC();
				//return;
				return;
			}
			DEBUG('a', "\n Finish reading filename.");
			
			if (!fileSystem->Create(filename, 0)) //Tao file bang ham Create cua fileSystem, tra ve ket qua
			{
				//Tao file that bai
				printf("\n Error create file '%s'", filename);
				machine->WriteRegister(2, -1);
				delete filename;
				//IncreasePC();
				//return;
				return;
			}
			
			//Tao file thanh cong
			machine->WriteRegister(2, 0);
			delete filename;
			//IncreasePC(); //Day thanh ghi lui ve sau de tiep tuc ghi
			//return;
}

void OpenHandler(){
	// Input: arg1: Dia chi cua chuoi name, arg2: type
			// Output: Tra ve OpenFileID neu thanh, -1 neu loi
			// Chuc nang: Tra ve ID cua file.
	 
			//OpenFileID Open(char *name, int type)
			int virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so name tu thanh ghi so 4
			int type = machine->ReadRegister(5); // Lay tham so type tu thanh ghi so 5
			char* filename;
			filename = User2System(virtAddr, MAX_FILE_LENGTH); // Copy chuoi tu vung nho User Space sang System Space voi bo dem name dai MAX_FILE_LENGTH
			//Kiem tra xem OS con mo dc file khong
			
			// update 4/1/2018
			int freeSlot = fileSystem->FindFreeSlot();
			if (freeSlot != -1) //Chi xu li khi con slot trong
			{
				if (type == 0 || type == 1) //chi xu li khi type = 0 hoac 1
				{
					
					if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL) //Mo file thanh cong
					{
						machine->WriteRegister(2, freeSlot); //tra ve OpenFileID
					}
				}
				else if (type == 2) // xu li stdin voi type quy uoc la 2
				{
					machine->WriteRegister(2, 0); //tra ve OpenFileID
				}
				else // xu li stdout voi type quy uoc la 3
				{
					machine->WriteRegister(2, 1); //tra ve OpenFileID
				}
				delete[] filename;
				return;
			}
			machine->WriteRegister(2, -1); //Khong mo duoc file return -1
			
			delete[] filename;
}

void CloseHandler(){
	//Input id cua file(OpenFileID)
			// Output: 0: thanh cong, -1 that bai
			int fid = machine->ReadRegister(4); // Lay id cua file tu thanh ghi so 4
			if (fid >= 0 && fid <= 14) //Chi xu li khi fid nam trong [0, 14]
			{
				if (fileSystem->openf[fid]) //neu mo file thanh cong
				{
					delete fileSystem->openf[fid]; //Xoa vung nho luu tru file
					fileSystem->openf[fid] = NULL; //Gan vung nho NULL
					machine->WriteRegister(2, 0);
					return;
				}
			}
			machine->WriteRegister(2, -1);
}

void ReadHandler() {
	// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte read thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Doc file voi tham so la buffer, so ky tu cho phep va id cua file
			int virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
			int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
			int id = machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6 
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 14)
			{
				printf("\nKhong the read vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the read vi file nay khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop doc file stdout (type quy uoc la 3) thi tra ve -1
			{
				printf("\nKhong the read file stdout.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = User2System(virtAddr, charcount); // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
			// Xet truong hop doc file stdin (type quy uoc la 2)
			if (fileSystem->openf[id]->type == 2)
			{
				// Su dung ham Read cua lop SynchConsole de tra ve so byte thuc su doc duoc
				int size = gSynchConsole->Read(buf, charcount); 
				System2User(virtAddr, size, buf); // Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su
				machine->WriteRegister(2, size); // Tra ve so byte thuc su doc duoc
				delete buf;
				IncreasePC();
				return;
			}
			// Xet truong hop doc file binh thuong thi tra ve so byte thuc su
			if ((fileSystem->openf[id]->Read(buf, charcount)) > 0)
			{
				// So byte thuc su = NewPos - OldPos
				NewPos = fileSystem->openf[id]->GetCurrentPos();
				// Copy chuoi tu vung nho System Space sang User Space voi bo dem buffer co do dai la so byte thuc su 
				System2User(virtAddr, NewPos - OldPos, buf); 
				machine->WriteRegister(2, NewPos - OldPos);
			}
			else
			{
				// Truong hop con lai la doc file co noi dung la NULL tra ve -2
				//printf("\nDoc file rong.");
				machine->WriteRegister(2, -2);
			}
			delete buf;
			IncreasePC();
}

void WriteHandler(){
	// Input: buffer(char*), so ky tu(int), id cua file(OpenFileID)
			// Output: -1: Loi, So byte write thuc su: Thanh cong, -2: Thanh cong
			// Cong dung: Ghi file voi tham so la buffer, so ky tu cho phep va id cua file
			int virtAddr = machine->ReadRegister(4); // Lay dia chi cua tham so buffer tu thanh ghi so 4
			int charcount = machine->ReadRegister(5); // Lay charcount tu thanh ghi so 5
			int id = machine->ReadRegister(6); // Lay id cua file tu thanh ghi so 6
			int OldPos;
			int NewPos;
			char *buf;
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 14)
			{
				printf("\nKhong the write vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the write vi file nay khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Xet truong hop ghi file only read (type quy uoc la 1) hoac file stdin (type quy uoc la 2) thi tra ve -1
			if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)
			{
				printf("\nKhong the write file stdin hoac file only read.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			OldPos = fileSystem->openf[id]->GetCurrentPos(); // Kiem tra thanh cong thi lay vi tri OldPos
			buf = User2System(virtAddr, charcount);  // Copy chuoi tu vung nho User Space sang System Space voi bo dem buffer dai charcount
			// Xet truong hop ghi file read & write (type quy uoc la 0) thi tra ve so byte thuc su
			if (fileSystem->openf[id]->type == 0)
			{
				if ((fileSystem->openf[id]->Write(buf, charcount)) > 0)
				{
					// So byte thuc su = NewPos - OldPos
					NewPos = fileSystem->openf[id]->GetCurrentPos();
					machine->WriteRegister(2, NewPos - OldPos);
					delete buf;
					IncreasePC();
					return;
				}
			}
			if (fileSystem->openf[id]->type == 3) // Xet truong hop con lai ghi file stdout (type quy uoc la 3)
			{
				int i = 0;
				while (buf[i] != 0 && buf[i] != '\n') // Vong lap de write den khi gap ky tu '\n'
				{
					gSynchConsole->Write(buf + i, 1); // Su dung ham Write cua lop SynchConsole 
					i++;
				}
				buf[i] = '\n';
				gSynchConsole->Write(buf + i, 1); // Write ky tu '\n'
				machine->WriteRegister(2, i - 1); // Tra ve so byte thuc su write duoc
				delete buf;
				IncreasePC();
				return;
			}
}

void SeekHandler(){
	// Input: Vi tri(int), id cua file(OpenFileID)
			// Output: -1: Loi, Vi tri thuc su: Thanh cong
			// Cong dung: Di chuyen con tro den vi tri thich hop trong file voi tham so la vi tri can chuyen va id cua file
			int pos = machine->ReadRegister(4); // Lay vi tri can chuyen con tro den trong file
			int id = machine->ReadRegister(5); // Lay id cua file
			// Kiem tra id cua file truyen vao co nam ngoai bang mo ta file khong
			if (id < 0 || id > 14)
			{
				printf("\nKhong the seek vi id nam ngoai bang mo ta file.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra file co ton tai khong
			if (fileSystem->openf[id] == NULL)
			{
				printf("\nKhong the seek vi file nay khong ton tai.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Kiem tra co goi Seek tren console khong
			if (id == 0 || id == 1)
			{
				printf("\nKhong the seek tren file console.");
				machine->WriteRegister(2, -1);
				IncreasePC();
				return;
			}
			// Neu pos = -1 thi gan pos = Length nguoc lai thi giu nguyen pos
			pos = (pos == -1) ? fileSystem->openf[id]->Length() : pos;
			if (pos > fileSystem->openf[id]->Length() || pos < 0) // Kiem tra lai vi tri pos co hop le khong
			{
				printf("\nKhong the seek file den vi tri nay.");
				machine->WriteRegister(2, -1);
			}
			else
			{
				// Neu hop le thi tra ve vi tri di chuyen thuc su trong file
				fileSystem->openf[id]->Seek(pos);
				machine->WriteRegister(2, pos);
			}
			IncreasePC();
			return;
}

void ExecHandler(){
	// Input: vi tri int
			// Output: Fail return -1, Success: return id cua thread dang chay
			// SpaceId Exec(char *name);
			int virtAddr;
			virtAddr = machine->ReadRegister(4);	// doc dia chi ten chuong trinh tu thanh ghi r4
			char* name;
			name = User2System(virtAddr, MAX_FILE_LENGTH + 1); // Lay ten chuong trinh, nap vao kernel
	
			if(name == NULL)
			{
				DEBUG('a', "\n Not enough memory in System");
				printf("\n Not enough memory in System");
				machine->WriteRegister(2, -1);
				//IncreasePC();
				return;
			}
			OpenFile *oFile = fileSystem->Open(name);
			if (oFile == NULL)
			{
				printf("\nExec:: Can't open this file.");
				machine->WriteRegister(2,-1);
				// IncreasePC();
				return;
			}

			delete oFile;

			// Return child process id
			int id = pTab->ExecUpdate(name); 
			machine->WriteRegister(2,id);

			delete[] name;	
			IncreasePC();
			return;
}

void JoinHandler(){
	// int Join(SpaceId id)
			// Input: id dia chi cua thread
			// Output: 
			int id = machine->ReadRegister(4);
			
			int res = pTab->JoinUpdate(id);
			
			machine->WriteRegister(2, res);
			IncreasePC();
			return;
}

void ExitHandler(){
	//void Exit(int status);
			// Input: status code
			int exitStatus = machine->ReadRegister(4);

			if(exitStatus != 0)
			{
				IncreasePC();
				return;
				
			}			
			
			int res = pTab->ExitUpdate(exitStatus);
			//machine->WriteRegister(2, res);

			currentThread->FreeSpace();
			currentThread->Finish();
			IncreasePC();
			return; 
}

char *User2System(int virtAddr, int limit)
{
	int i; // index
	int oneChar;
	char *kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf, 0, limit + 1);
	//printf("\n Filename u2s:");
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

// Input: - User space address (int)
// - Limit of buffer (int)
// - Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr, int len, char *buffer)
{
	if (len < 0)
		return -1;
	if (len == 0)
		return len;
	int i = 0;
	int oneChar = 0;
	do
	{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

// Input: None
// Output: None
// Purpose: Increase program counter
void IncreasePC()
{
	machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}