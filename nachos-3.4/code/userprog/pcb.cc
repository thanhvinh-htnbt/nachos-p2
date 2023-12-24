#include "pcb.h"
#include "utility.h"
#include "system.h"
#include "thread.h"
#include "addrspace.h"

extern void StartProcessWithID(int id);

PCB::PCB(int id) {
    if (id == 0)
        parentID = -1;
    else
        parentID = currentThread->processID;

    numwait = exitcode = boolBG = 0;
    thread = NULL;

    joinsem = new Semaphore("joinsem", 0);
    exitsem = new Semaphore("exitsem", 0);
    multex = new Semaphore("multex", 1);
}

PCB::~PCB() {
    if (joinsem != NULL)
        delete joinsem;
    if (exitsem != NULL)
        delete exitsem;
    if (multex != NULL)
        delete multex;
    if (thread != NULL) {
        thread->FreeSpace();
        thread->Finish();
    }
}

int PCB::GetID() {
    return thread->processID;
}

int PCB::GetNumWait() {
    return numwait;
}

int PCB::GetExitCode() {
    return exitcode;
}

void PCB::SetExitCode(int ec) {
    exitcode = ec;
}

void PCB::JoinWait() {
    // Process translation to block - Wait for JoinRelease to continue exec
    joinsem->P();
}

void PCB::JoinRelease() {
    // JoinRelease process calling JoinWait
    joinsem->V();
}

void PCB::ExitWait() {
    // Let process translation to block state - Waiting for ExitRelease to continue exec
    exitsem->P();
}

void PCB::ExitRelease() {
    // Release waiting process
    exitsem->V();
}

void PCB::IncNumWait() {
    multex->P();
    ++numwait;
    multex->V();
}

void PCB::DecNumWait() {
    multex->P();
    if (numwait > 0)
        --numwait;
    multex->V();
}

void PCB::SetFileName(char* fn) {
    strcpy(FileName, fn);
}

char* PCB::GetFileName() {
    return FileName;
}

int PCB::Exec(char* filename, int id) {
    // Use mutex->P(); to prevent loading two processes simultaneously
    multex->P();

    // Check if the thread has been successfully initialized. If not, report an out-of-memory error, call mutex->V(), and return.
    thread = new Thread(filename);

    if (thread == NULL) {
        printf("\nPCB::Exec:: Not enough memory..!\n");
        multex->V();
        return -1;
    }

    // Set the processID of this thread to id
    thread->processID = id;
    // Set the parentID of this thread to the processID of the calling thread executing Exec
    parentID = currentThread->processID;
    // Execute Fork(StartProcessWithID, id) - Cast the thread to int, and later cast Thread back to its correct type in the StartProcess function.
    thread->Fork(StartProcessWithID, id);

    multex->V();
    // Return id
    return id;
}
