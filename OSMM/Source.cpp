#include<iostream>
#include<string>
#include<list>
#include<queue>
#include<algorithm>
using namespace std;
constexpr auto MMsize = 10;
string VM[MMsize * 2];//virtual memory
string MM[2][MMsize];//main memory


//Entry for Page table
struct ptEntry
{
	int frameNo = -1;
	string pageNo;
	bool validBit = false;
};

//process control block
struct PCB
{
	string name = "";
	int size;
	int base;
	int limit;
	//should be a reference to pagetable of the process
	//but the table kept getting destroyed after exiting the scope so i put the whole table here
	list<ptEntry> pageTable;
};

//TLB entry
struct TLBEntry
{
	string process;
	string pageNo;
	int frameNo;
};

//list of all the process PCBs
list<PCB> PCBlist; 
//oldest index inserted in the main memory
int oldestIndex = 0;
//Transition Lookaside Buffer
deque<TLBEntry> TLB;


int findEmptyFrame();
void loadPage(PCB & pcb, string pname);
int findPageVM(string pno, string pname);
void TLBpush(string pname, string pno, int frameNo);
bool TLBcheck(string pname, string pno);
PCB getPCB(string pname);

//load process in Main memory(RAM) 
//creates PCB and pageTable
void loadProcess(string pname)
{
	char ch;
	PCB pcb = getPCB(pname);
	list<ptEntry> pageTable;
	if(pcb.name==pname)
	{
		cout << "Process already running.." << endl;
	}
	else
	{
		pageTable = pcb.pageTable;
		cout << "Creating process" << endl;
		
		int index = 0, frame;
		while (VM[index] != pname)
		{
			index++;
		}
		frame = findEmptyFrame();
		MM[0][frame] = pname;
		MM[1][frame] = VM[index];

		pcb.name = pname;
		//pcb.pageTable = pageTable;
		

		ptEntry entry;
		entry.frameNo = frame;
		entry.pageNo = VM[index];
		entry.validBit = true;
		index++;
		pageTable.push_back(entry);
		while (VM[index] != "-1")
		{
			ptEntry e;
			e.pageNo = VM[index];
			index++;
			pageTable.push_back(e);
		}
		pcb.pageTable = pageTable;
		PCBlist.push_back(pcb);
	}
	do
	{
		cout << "Do you want to load pages into the memory?? (y/n)"; cin >> ch;
		if (ch == 'y')
		{
			loadPage(pcb, pname);
		}
	} while (ch == 'y');
}
void loadPage(PCB & pcb,string pname)
{
	string pno;
	
	for (list<ptEntry>::iterator it = pcb.pageTable.begin(); it != pcb.pageTable.end(); it++)//printing all the pages of the process
	{
		cout << it->pageNo << endl;
	}
	list<ptEntry>::iterator itr = pcb.pageTable.begin();
	cout << "Enter Page No. you want to access: "; cin >> pno;
	if (TLBcheck(pname,pno)==true)
	{

	}
	else
	{
		while (itr != pcb.pageTable.end() && itr->pageNo != pno)
		{
			//cout << itr->pageNo << endl;
			itr++;
		}
		if (itr->pageNo == pno)
		{
			if (itr->validBit == true)
			{
				cout << "Already in main memory..";
				cout << "Showing... " + MM[1][itr->frameNo];
				TLBpush(pname, pno, itr->frameNo);
			}
			else
			{
				cout << endl << "Page not in Main memory loading it from the secondary drive...." << endl;
				int index = findPageVM(pno, pname);
				int frame = findEmptyFrame();
				MM[0][frame] = pname;
				MM[1][frame] = VM[index];
				itr->frameNo = frame;
				itr->validBit = true;
				TLBpush(pname, pno, frame);
				cout << "Success!!" << endl;
			}
		}
	}
}

PCB getPCB(string pname)
{
	list<PCB>::iterator itr;
	PCB pcb;
	for (itr = PCBlist.begin(); itr != PCBlist.end(); itr++)
	{
		if (itr->name == pname)
		{
			pcb = *itr;
			return pcb;
		}
	}
	return pcb;
}

void TLBpush(string pname,string pno,int frameNo)
{
	TLBEntry buffer;
	buffer.process = pname;
	buffer.pageNo = pno;
	buffer.frameNo = frameNo;
	if (TLB.size() == (MMsize / 5))
	{
		TLB.pop_front();
		TLB.push_back(buffer);
	}
	else
	{
		TLB.push_back(buffer);
	}
}

bool TLBcheck(string pname, string pno)
{
	cout << "Checking for the page in TLB" << endl;
	deque<TLBEntry>::iterator itr;
	for (itr = TLB.begin(); itr != TLB.end() && (itr->process != pname || itr->pageNo != pno); itr++);
	if (itr == TLB.end())
	{
		cout << "Page not Found in TLB.." << endl;
		return false;
	}
	else if (itr->process == pname && itr->pageNo == pno)
	{
		cout << endl << "Page found in TLB!" << endl;
		cout << "Precoess: " << itr->process << " Page: " << itr->pageNo << " FrameNo: " << itr->frameNo << endl;
		return true;
	}
	return false;
}

int FIFOreplacement()
{
	cout << "Memory is full using page replacement algorithem" << endl;
	int temp = oldestIndex;
	string pname = MM[0][oldestIndex];
	string page = MM[0][oldestIndex];
	list<PCB>::iterator itr;
	for (itr = PCBlist.begin(); itr != PCBlist.end() && itr->name != pname; itr++);
	list<ptEntry> table = itr->pageTable;
	list<ptEntry>::iterator it;
	for (it = table.begin(); it != table.end() && it->pageNo != page; it++);
	it->frameNo = -1;
	it->validBit = false;
	MM[0][oldestIndex] = "-1";
	MM[1][oldestIndex] = "-1";
	oldestIndex=(oldestIndex+1)%MMsize;
	cout << "Successfully Emptied index " << temp << " of memory" << endl;
	return temp;
}
int findPageVM(string pno,string pname)
{
	int index = 0;
	while (VM[index] != pname)
	{
		index++;
	}
	while (VM[index] != pno)
	{
		index++;
	}
	return index;
}
int findEmptyFrame()
{
	int i = 0;
	while (MM[1][i] != "-1" && i<MMsize)
	{
		i++;
	}
	if (i == MMsize)
	{
		return FIFOreplacement();
	}
	else
		return i;

}
//functioon to fill the virtual memory
void fillVM()
{
	int nop = 0;//no of programs
	int psize = 0;//size of program
	int index=0,pageNo;
	cout << "Enter NO of programs: "; cin >> nop;
	for (int i = 0; i < nop; i++)
	{
		cout << "Enter size of P" << i << ": ";
		cin >> psize;
		VM[index] = "P" + to_string(i);
		pageNo = 1;
		for (int j = index+1; pageNo < psize; j++)
		{
			//cout << "page" << pageNo << endl;
			VM[j] = "page" + to_string(pageNo);
			index++; pageNo++;
		}
		index++;
		VM[index] = "-1";
		index++;
	}
}
void initializeMemory()
{
	for (int i = 0; i < MMsize*2; i++)
	{
		VM[i] = "-1";
	}
	for (int i = 0; i < MMsize; i++)
	{
		MM[1][i] = "-1";
	}
	PCB pcb;
	PCBlist.push_back(pcb);
}
void printMemory()
{
	cout << endl;
	cout << "printing virtual memory" << endl;
	for (int i = 0; i < (MMsize * 2); i++)
	{
		cout << VM[i]+" ";
	}

	cout <<endl<< "printing main memory" << endl;
	for (int i = 0; i < MMsize; i++)
	{
		cout << MM[1][i] + " ";
	}
}
void printProgramList()
{
	int i=0,j=0;
	string pname= "P" + to_string(j);
	while (i < MMsize * 2)
	{
		if (VM[i] == pname)
		{
			pname = "P" + to_string(j);
			j++;
			
			cout << pname <<" " ;
			pname = "P" + to_string(j);
		}
		i++;
	}
}
int main()
{
	char choice;
	string pname;
	initializeMemory();
	fillVM();
	do {
		cout<< flush;
		system("CLS");
		cout << "All the programms currently available" << endl;
		printProgramList();
		cout << endl << "Select a program to load" << endl;
		cin >> pname;
		loadProcess(pname);
		
		cout << "1. Switch process" << endl << "0. Exit" << endl;
		cin >> choice;
	}
	while (choice != '0');
	printMemory();
	system("pause");
}

