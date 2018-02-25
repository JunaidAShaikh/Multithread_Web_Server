   
/*
 * soc.c - program to open sockets to remote machines
 *
 * $Author: kensmith $
 * $Id: soc.c 6 2009-07-03 03:18:54Z kensmith $
 */

static char svnid[] = "$Id: soc.c 6 2009-07-03 03:18:54Z kensmith $";

#define	BUF_LEN	8192
#include <iostream>
#include <time.h>
#include <stack>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<inttypes.h>
#include    <unistd.h>
#include    <cstdlib>
#include    <queue>
#include <fstream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <thread>        
#include <vector> 
#include <sstream>
#include <fstream>
#include <list>
#include<mutex>
#include <pthread.h>
#include <condition_variable>
#include <chrono>
#include <time.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include<iostream>
#include <semaphore.h>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>

using namespace std;
string tem_port = "8080";
const char *port = tem_port.c_str();

bool daemonss = false;
bool logging = false;
string scheduling ="FCFS";
int threadnum;
bool summary = false;
int temp_time = 60;
string logfile = "";
char ch;
string rootdir = "."; 
int s, sock, server, bytes, aflg;
int soctype = SOCK_STREAM;
char *host = NULL;
bool schedule_done = false;

void *thread_listen(struct sockaddr_in* arg);

sem_t request_count;

mutex request_queue_mutex;
ofstream logging_file;

string getDate() {
    string current_month;
    int month;
	time_t current_time;
	stringstream sstm;
	string total;
	struct tm *pointer;

	time(&current_time);
	pointer = gmtime(&current_time);

	month = (pointer->tm_mon) + 1;

	switch(month) {
	    case 1: current_month = "Jan"; break;
	    case 2: current_month = "Feb"; break;
	    case 3: current_month = "Mar"; break;
	    case 4: current_month = "Apr"; break;
	    case 5: current_month = "May"; break;
	    case 6: current_month = "Jun";break;
	    case 7: current_month = "Jul";break;
	    case 8: current_month = "Aug";break;
	    case 9: current_month = "Sep";break;
	    case 10: current_month = "Oct";break;
	    case 11: current_month = "Nov";break;
	    case 12: current_month = "Dec";break;
	}
	
     
    sstm << "Date:" << pointer->tm_mday << "/" << current_month << "/" << ((pointer->tm_year)%100)+2000 << ":" << pointer->tm_hour << ":" << (pointer->tm_min)%60 << ":" << pointer->tm_sec << " " << "-0400" << "\n";
    total = sstm.str();
	return total;
}


string get_lastmodified(string fileName) {
   
	const char * c = fileName.c_str();
	 struct tm *tmModifiedTime;
     struct stat attrib;
    char buffer[80];
    stat(c, &attrib);
	 tmModifiedTime = gmtime(&attrib.st_mtime);
	 strftime(buffer,sizeof(buffer),"%d/%b/%Y:%I:%M:%S -0400",tmModifiedTime);
	 string str(buffer);
	 return str; 
}

uint64_t get_gtod_clock_time ()
{
    struct timeval tv;

    if (gettimeofday (&tv, NULL) == 0)
        return (uint64_t) (tv.tv_sec * 1000000 + tv.tv_usec);
    else
        return 0;
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char      buf[80];
    tstruct = *gmtime(&now);
    strftime(buf, sizeof(buf), "%d/%b/%Y:%X", &tstruct);
	string str = string(buf) + " -4:00";
    return str;
}

class Request
{
public:
    bool isGet = false;
    bool isError = false;
    string fileName = "";
    string contentType = "";
    int fileSize = 0;
	string request_time = "";
	string arrival_time = "";
	string last_modified = "";
	string status_line = ""; 
	string status_code = "";
	int clientid = -1;
	string request_string = "";
	string service_time = "";
	string ip = "";
	bool isDirectory = false;
	string directoryName = "";
	bool isDirectoryListing = false;
	string pathName = ""; 
	string extension = "";

public:
    Request(string toParse, int cid) // not a default constructor
    {
		clientid = cid;
		request_string = toParse;
		
	request_time = getDate();
    if(toParse.substr(0,toParse.find(" "))== "GET")
        isGet = true;
	
	
	if(!isGet) {
		fileSize = 0;
	}
    toParse = toParse.substr(toParse.find(" ")+1, toParse.length());

    fileName = toParse.substr(0, toParse.find(" "));

    if(fileName.substr(0,1) == "/")
        fileName=fileName.substr(1,fileName.length());

	if((fileName.find("."))!=string::npos){
		//cout<<"Extension exists"<<endl;
		extension = fileName.substr(fileName.find(".")+1,fileName.length());
	}
	
	//cout<< "Extension: "<< extension << endl;

    if(extension == "txt" || extension == "html")
        contentType = "text/html";
	else if(extension == ""){ 
		isDirectory = true;
		directoryName=fileName;
		fileName="/index.html";
		//cout << "Directory Name" << directoryName << endl;
		//cout << "File Name" << fileName << endl;
		contentType = "text/html";
	}
    else 
        contentType = "image/gif";

	if(isDirectory)
	{
		if(opendir((rootdir+"/"+directoryName).c_str()) == NULL){
			isError = true;
			status_line = "HTTP/1.0 404 DirectoryNotFound";
			status_code = "404";
		}
		else{
		ifstream myfile ((rootdir+"/"+directoryName+fileName).c_str());
		if (!myfile.is_open())
		{
			isDirectoryListing = true;
			cout << "Directory Listing: " << isDirectoryListing << endl;
			fileSize = 0;
		}
		}
	}
	
	if(isDirectory) {
		pathName = rootdir +"/"+ directoryName +fileName;
	cout << "Path Name:"; }
	else 
		pathName = rootdir +"/"+ fileName; 
	
	    cout << "trying to open path name " << pathName << endl;
		ifstream ifs (pathName.c_str()); 


    if(ifs.is_open()){
		cout<<"Found file: "<<fileName.c_str()<<endl;
		cout<<"Found file: "<<directoryName<<"/"<<fileName.c_str()<<endl;
        ifstream fs ((rootdir+"/"+directoryName+fileName).c_str(), ifstream::ate | std::ifstream::binary);
		int temp_file = 0;
        fileSize = fs.tellg();
        
		status_line = "HTTP/1.0 200 OK"; 
		status_code = "200";

		//cout<< "Server: "  << "\n";
		last_modified = get_lastmodified(fileName);
		//cout<< "Content-Type: " << contentType << "\n";
        //cout<< "Content-Length: " << fileSize << "\n";
		//if (isGet) {
			//cout << "\n";
			//cout << ifs.rdbuf(); }
    }
    else if(!isDirectory){
		cout<<"could not Found file: "<<fileName.c_str()<<endl;
		cout<<"could not Found file: "<<directoryName<<"/"<<fileName.c_str()<<endl;
		status_line = "HTTP/1.0 404 FileNotFound";
	    status_code = "404";
		fileSize = 0;
        isError = true; 
    }
}
};


list<Request> request_list;
std::list<std::thread> threads;
mutex request_mutex;
condition_variable threads_cv; 

  
typedef bool(*CompareFunc)(Request, Request);

bool reqCompare(Request i_lhs, Request i_rhs) {
	//cout<<"Left: "<<i_lhs.fileSize<< " Right: "<<i_rhs.fileSize<<endl;
	bool y = (i_lhs.fileSize < i_rhs.fileSize);
	//cout<<"Return value: "<< y <<endl;
return (i_lhs.fileSize < i_rhs.fileSize);}




bool getdir (string dir, list<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        //cout << "Error - cannot open directory" ;
        return false;
    }

    while ((dirp = readdir(dp)) != NULL) {
		
        files.push_back(string(dirp->d_name));
		
    }
    closedir(dp);
	return true;
}


bool requestAvaliable() {return request_list.size() != 0;}

void processRequest() {
	sleep(temp_time);
	std::thread::id this_id = std::this_thread::get_id();
	//cout << "Thread started id: " << this_id << "Thread started time: " << get_gtod_clock_time() << endl;
	while(true) {
		//cout << "Checking the mutex value" << endl;
		sem_wait(&request_count);
		request_queue_mutex.lock();
	   //cout << "Waiting for a mutex" << this_id << endl;
	//cout << "Waiting for a request avaliable" << endl;
		Request temp = request_list.front();
		request_list.pop_front();
		//cout<<"Got object: "<<temp.fileName<<" with size: "<<temp.fileSize<<endl;
		request_queue_mutex.unlock();
		temp.service_time = currentDateTime();
		//cout << "Service time " << temp.service_time << endl;
		cout << "Servicing: "<<temp.fileName<<" id: " << this_id << "Serive Time: " << get_gtod_clock_time() << endl;
		//cout << threads.front() 
		logging_file << temp.ip << " " << "[" << temp.arrival_time << "] " << "[" << temp.service_time << "] " << "\"" << temp.request_string << "\""  << " "<< temp.status_code << " " << temp.fileSize << endl; 
		//cout << "before fileName"<<endl;
		//cout << "fileName: "<<temp.fileName << endl;
		//cout << "between fileName and fileSize" << endl;
		//cout << "fileSize" << (temp.fileSize).str() << endl;
		
		
		/*ifstream ifs("red.jpg", ios::in | ios::binary); // input file
        ostringstream oss;	// output to string
	    ofstream fout("test.jpg");
	    oss << ifs.rdbuf();
	    string data = oss.str();
	    cout << data<<endl;
	    fout << data; */
		
		//directory listing 
		
	
        
		string headerString = temp.status_line + "\n" +
                   temp.request_time + 
				   + "Last-Modified:" + temp.last_modified + "\n"
				   + "Content-Type:" + temp.contentType + "\n" 
		           + "Content-Length:" + to_string(temp.fileSize) + "\n"; 
				   
		//cout<<" headerString: "<<headerString<<endl;
	    
		const char *head = headerString.c_str();	
		list<string> files;
		if(!temp.isError){
				cout<<"reached isError"<<endl;
			if(temp.isDirectory)
			{
				cout<<"reached isDirectory"<<endl;
				if(temp.isDirectoryListing)
				{
					cout<<"reached isDirectoryListing"<<endl;
				getdir(rootdir+"/"+temp.directoryName,files);
				files.sort();
				while(files.front().substr(0,1) == ".") {
							files.pop_front(); }
				string listOfFilesString="";
				
				for(list<string>::iterator it = files.begin();it != files.end();++it)
				{
						listOfFilesString=listOfFilesString+(*it)+"\n";
				}
				const char *listOfFiles = listOfFilesString.c_str();	
				send(temp.clientid, listOfFiles, strlen(listOfFiles),0);
				}
				else if(!temp.isGet)
					send(temp.clientid,head, strlen(head),0); //Dierect Head Request
				else{
					//read directory+index.html into content
					ifstream ifs (temp.pathName.c_str(), std::ifstream::binary);
					
						ifs.seekg(0, ifs.end);
						int length = ifs.tellg();
						ifs.seekg(0, ifs.beg);
						char* content =  new char[length];
					    memset(&content[0], 0, sizeof(content));
						ifs.read (content,length);
						ifs.close();
					char* total_response = new char[strlen(head) + strlen("\n") + strlen(content)];
					strcpy(total_response,head);
					strcat(total_response,"\n");
					strcat(total_response,content);
					//cout << "THis is where" << endl;
					send(temp.clientid,total_response, strlen(total_response),0);
					cout << "Content:" << content << endl;
					cout << "total_response" << total_response << endl; 
					delete[] content;
					delete[] total_response;
				}
			}
			else if(temp.isGet){
				//read temp.fileName into content
				ifstream ifs (temp.pathName.c_str(), std::ifstream::binary);
					cout<<"was able to open: "<<temp.pathName.c_str()<<endl;
					ifs.seekg(0, ifs.end);
					int length = ifs.tellg();
					//cout<<" length: "<<length;
					ifs.seekg(0, ifs.beg);
					char* content = new char[length];
					memset (content, ' ', sizeof(content));
					cout<<" pre read: "<<content<<endl;
					ifs.read (content,length);
						ifs.close();			
						cout<<" post read: "<<content<<endl;
					//cout << "GO total_Response:"  << head << endl;
				char* total_response = new char[strlen(head) + strlen("\n") + strlen(content)];
			 	strcpy(total_response,head);
				strcat(total_response,"\n");
				strcat(total_response,content);
				send(temp.clientid,total_response, strlen(total_response),0);
				cout << "total_response" << total_response << endl; 
					delete[] content;
					delete[] total_response;
			}
			else {
				//cout << "GO inside:"  << head << endl;
			send(temp.clientid,head, strlen(head),0); }//File Head Request
			cout << "Head" << head << endl; 
		}
		else
			send(temp.clientid,"404 FileNotFound HTTP/1.0", strlen("HTTP/1.0 404 FileNotFound"),0);				
}
}
void createThreadPool(int n) {
	
	for (int i=1; i<=n; ++i) {
      threads.push_back(thread(processRequest));
	}
}

std::list<Request>::iterator it;

void *thread_listen(struct sockaddr_in* arg)
{
	int client_id,x;
	struct sockaddr_in *remote =(struct sockaddr_in *)arg;
	socklen_t len = sizeof(remote);
	char buf[1024];
	char ip[INET_ADDRSTRLEN];
	while(true)
	{
		client_id = accept(s, (struct sockaddr *) &remote, &len);
		inet_ntop(AF_INET, &remote, ip, sizeof(ip));
		//cout<<"client_id"<<client_id<<"\n";
		string buff_string = "";
		x=read(client_id,buf,1023);
		buff_string = string(buf);
		//cout<<" received: "<<buff_string<<"\n";
		Request temp(buff_string,client_id);
		request_queue_mutex.lock();
		string str(ip);
		temp.ip = str;
		temp.arrival_time = currentDateTime();
		request_list.push_back(temp);
        cout << "Arrivial time " << get_gtod_clock_time() << endl;
		if(scheduling == "SJF") {
			//cout << "I am sorting" << endl;
			request_list.sort(reqCompare);
		}
		for (it=request_list.begin(); it!=request_list.end(); ++it) {
              std::cout << ' ' << (*it).fileSize;
		std::cout << '\n'; }
        request_queue_mutex.unlock();
        sem_post(&request_count);
		//COMMENT: This is where you get the request. Create the request object here and include client_id parameter in it.
		//getRequest(buff_string);
	}
	//COMMENT: Use the following part where you need to send the reply to the client while serving the request. 
	//COMMENT: Change it from client_id to request_object.client_id
	/*string y = "blah";
	char *chary = new char[y.length()+1];
	strcpy(chary,y.c_str());
	send(client_id,chary, strlen(chary),0); */
}

int main(int argc,char *argv[])
{	
	
while ((ch = getopt(argc, argv, "dhl:p:r:t:n:s:")) != -1)
	{
		switch( ch ) {
			case 'd':
			daemonss = true;
            break;
               
            case 'h':
                summary = true;
                break;
                    
            case 'l':
                logfile = optarg;
                logging = true;
                break;
                    
            case 'p':
                port = optarg;
                break;
                    
            case 'r':
                rootdir = optarg;
                break;
                    
            case 's':
                scheduling = optarg;
                break;
                    
            case 't':
                temp_time =atoi(optarg);
                break;
                    
            case 'n':
                threadnum=atoi(optarg);
                break;
                    
            default:
			break;
        }
       // ch = getopt( argc, argv, "dhl:p:r:t:n:s:" );
            
	}

    // now we loop back and get the next line in 'str'
	
	
	if(summary){
 
	cout<<"Usage Summary: myhttpd -d -h -l filename -p portno -r rootdirectory -t time -n threadnumber -s scheduling"<<endl<<endl;
	cout<<"Give -d parameter to accept only one condition"<<endl;
	cout<<"Give -h parameter to display the summary"<<endl;
	cout<<"Give -l to log all requests into a given file say LOG.txt"<<endl;
	cout<<"Give -p followed by port number , by default myhttpd will listen on 8080"<<endl;
	cout<<"Give -r and then root directory for the http server to dir"<<endl;
	cout<<"Give -t to set the queuing time by default the http server should be 60 seconds "<<endl;
	cout<<"Give -n and then thread numbers to change the default value of threads for example: -n 10"<<endl;
	cout<<"Give -s and then scheduling name to change default scheduling which is FCFS to -s SJF"<<endl;
	return 0;

}

    sem_init(&request_count,0,0);

    logging_file.open (logfile + ".txt", ios::app);
	
		
	
	struct sockaddr_in serv, remote;
	struct servent *se;
	int newsock;
	socklen_t len;
	char *port = NULL;
	
	if ((s = socket(AF_INET, soctype, 0)) < 0) {
		perror("socket");
		exit(1);
	}
	len = sizeof(remote);
	memset((void *)&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	if (port == (char *) NULL){
		serv.sin_port = htons(0);
	}
	else if (isdigit(*port))
		serv.sin_port = htons(atoi(port));
	else {
		if ((se = getservbyname(port, (char *)NULL)) < (struct servent *) 0) {
			perror(port);
			exit(1);
		}
		serv.sin_port = se->s_port;
	}
	if (bind(s, (struct sockaddr *)&serv, sizeof(serv)) < 0) {
		perror("bind");
		exit(1);
	}
	if (getsockname(s, (struct sockaddr *) &remote, &len) < 0) {
		perror("getsockname");
		exit(1);
	}
	fprintf(stderr, "Port number is %d\n", ntohs(remote.sin_port));
	listen(s, 1);
	newsock = s;
	

	thread listener(&thread_listen, &remote);
	
	if(daemonss) {
		createThreadPool(1);
		ifstream in(logfile + ".txt");
	    string str;
      while (getline(in, str)) {
    	cout << str << endl;}
		}
	else {
		createThreadPool(threadnum);
	}
	
	
	//cout << "I am done scheduling" << schedule_done<<endl;
	//cout<<"Schedule is: "<<scheduling<<endl;

   
 
   for(auto& thread : threads)  {
      thread.join(); 
	  }
	  

	//COMMENT: You can put the listener.join near the end of our main function
	logging_file.close(); 
	listener.join();
}   

		










