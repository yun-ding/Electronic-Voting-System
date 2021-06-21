#include <iostream>
#include <iomanip>
#include <thread>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>
#include <string.h> 
#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <typeinfo>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_t tid;

bool shutdown_req = false;

std::string default_password = "cit595";
std::string default_port = "10000";   

std::vector<std::string> candidate_list;
std::vector<int> count_list;  
std::vector<int> voter_list;
std::vector<int> magicno_list;

bool exists = false;

char receive_buffer[1024];
char send_buffer[1024];

std::string start_election(const std::string& password){
    pthread_mutex_lock(&lock1);
    
    if(exists == false){
        exists = true;
        
        candidate_list.clear();
        count_list.clear();  
        voter_list.clear();
        magicno_list.clear();
        
        pthread_mutex_unlock(&lock1);
		return "OK";
	}else{
        pthread_mutex_unlock(&lock1);
		return "EXISTS";
		}
    
	pthread_mutex_unlock(&lock1);
	return "ERROR"; 	
}

std::string add_candidate(const std::string& password, std::string candidate){
    pthread_mutex_lock(&lock1);
    
    if (exists == false){
        pthread_mutex_unlock(&lock1);
        return "ERROR";
    }
    
    candidate.erase(std::remove(candidate.begin(), candidate.end(), '\n'), candidate.end());
    
    if (std::find(candidate_list.begin(), candidate_list.end(), candidate) != candidate_list.end()){
        pthread_mutex_unlock(&lock1);
        return "EXISTS";
    }else{
        
        candidate_list.push_back(candidate);
        count_list.push_back(0);
        pthread_mutex_unlock(&lock1);
        return "OK";
    }
}

std::string shutdown(const std::string& password){
    if (exists == false){
        return "ERROR";
    }
    
    pthread_mutex_lock(&lock1);
    
    ofstream file;
    file.open("backup.txt");
    file << "VOTING RESULT:" << "\n";
    for (unsigned int i=0; i<candidate_list.size(); i++){
        file << candidate_list[i] << ":" << count_list[i] << "\n";
    }
    
    file << "VOTERS LIST:" << "\n";
    for (unsigned int i=0; i<voter_list.size(); i++){
        file << voter_list[i] << "\n";
    }
    
    file << "MAGICNO LIST:" << "\n";
    for (unsigned int i=0; i<magicno_list.size(); i++){
        file << magicno_list[i] << "\n";
    }
    file.close();
    
    ofstream file2;
    file2.open("password.txt");
    file2 << default_password << "\n";
    file2.close();
                    
    ofstream file3;
    file3.open("port.txt");
    file3 << default_port << "\n";
    file3.close();
 
    shutdown_req = true;
    
    std::string default_password = "cit595";
    std::string default_port = "10000";
    
    pthread_mutex_unlock(&lock1);
    
	return "OK";
}

std::string add_voter(int voterid){
    if (exists == false){
        return "ERROR";
    }
    
    if (voterid < 1000 || voterid > 9999){
        return "ERROR";
    }
    
    if (std::find(voter_list.begin(), voter_list.end(), voterid) != voter_list.end()){
        return "EXISTS";
    }else{
        pthread_mutex_lock(&lock1);
        voter_list.push_back(voterid);
        pthread_mutex_unlock(&lock1);
        return "OK";
    }
}

std::string check_registration_status(int voterid){
    if (exists == false){
        return "ERROR";
    }
    
    if (voterid<1000 || voterid>9999){
        return "INVALID";
    }else if (std::find(voter_list.begin(), voter_list.end(), voterid) != voter_list.end()){
        return "EXISTS";
    }else{
        return "UNREGISTERED";
    }
}

std::string check_voter_status(int voterid, int magicno){
    if (exists == false){
        return "ERROR";
    }
    
    if (std::find(voter_list.begin(), voter_list.end(), voterid) == voter_list.end()){
        return "CHECKSTATUS";
    }else{
        if (std::find(magicno_list.begin(), magicno_list.end(), magicno) != magicno_list.end()){
            return "ALREADYVOTED";
        }else{
            return "UNAUTHORIZED";
        }
    }
}

std::string vote_for(std::string name, int voterid){
    if (exists == false){
        return "ERROR";
    }
    
    name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
    
    if (check_registration_status(voterid).compare("EXISTS") != 0){
        return check_registration_status(voterid);
    }
    
    pthread_mutex_lock(&lock1);
    int candidate_id = -1;
    for (unsigned int i=0; i<candidate_list.size(); i++){
        std::string curr = candidate_list.at(i);
        const char *ccurr = curr.c_str();
        const char *cname = name.c_str();
        if (strcmp(ccurr, cname) == 0){
            candidate_id = i;
        }
    }
    pthread_mutex_unlock(&lock1);
		
    int magicno;
    if (candidate_id != -1){
        std::string s1 = std::to_string(voterid); 
        std::string s2 = std::to_string(candidate_id);
        std::string s3 = s1 + s2;
        magicno = stoi(s3);
        
        if (check_voter_status(voterid, magicno).compare("UNAUTHORIZED") == 0){
            pthread_mutex_lock(&lock1);
            int new_count = count_list[candidate_id] + 1;
            count_list[candidate_id] = new_count;
            magicno_list.push_back(magicno);
            pthread_mutex_unlock(&lock1);
            
            std::string result = "EXISTS\n";
            result += s3;
            return result;
        }else{	
            return "ERROR";
        }
    }else{
        pthread_mutex_lock(&lock1);
        candidate_list.push_back(name);
        count_list.push_back(1);
		std::string s1 = std::to_string(voterid); 
        std::string s2 = std::to_string(candidate_list.size()-1);
        std::string s3 = s1 + s2;
        magicno = stoi(s3);
        
        magicno_list.push_back(magicno);
        pthread_mutex_unlock(&lock1);
        
        std::string result = "NEW\n";
        result += s3;
        return result;
    }
}

std::string list_candidates(){
    if (exists == false){
        return "ERROR";
    }
    
    pthread_mutex_lock(&lock1);
    
    std::string res = "\"<";
    for (unsigned int i = 0; i < candidate_list.size()-1; i++) {
        res += candidate_list[i];
        res += ">\n<";
    }
    res += candidate_list[candidate_list.size()-1];	
    res += ">\n";
    res += "\"";
    
    pthread_mutex_unlock(&lock1);
    
    return res;
}

std::string vote_count(std::string name){
    if (exists == false){
        return "-1";
    }
    
	std::string count = "-1";
    
    pthread_mutex_lock(&lock1);
    
    for (unsigned int i = 0; i < count_list.size(); i++) {
        if (candidate_list[i].compare(name) == 0){
			count = std::to_string(count_list[i]);
        }
    }
    
    pthread_mutex_unlock(&lock1);
    
    return count;
}

std::string view_result(){
    pthread_mutex_lock(&lock1);
    
    std::vector<int> winner_list;
    std::string res = "";
    int max_count = 0; 
    for (unsigned int i = 0; i < count_list.size(); i++) {
            if (count_list[i] > max_count) {
                winner_list.clear();
                max_count = count_list[i];
                winner_list.push_back(i);
            }else if (max_count != 0 && count_list[i] == max_count){
                winner_list.push_back(i);
            }
        res += "<";
        res += candidate_list[i];
        res += ">:<";
        res += std::to_string(count_list[i]);
        res += ">\n";
    }
    
    if (winner_list.size() == 1){
        res += "Winner:<";
        res += candidate_list[winner_list[0]];
        res += ">";
    }else if (winner_list.size() == 0){
        res += "No Winner";
    }else{
        res += "Draw:<";
        for (unsigned int i = 0; i < winner_list.size()-1; i++) {
            res += candidate_list[winner_list[i]];
            res += ">,<";
        }
        res += candidate_list[winner_list[winner_list.size()-1]];
        res += ">";
    }
    
    pthread_mutex_unlock(&lock1);
    
	return res;
}

std::string end_election(const std::string& password){
    if (exists == false){
        return "ERROR";
    }
    
    std::string res = view_result();
	exists = false;
	return res;
}

void* function(){
		// get the input
		vector<char*> command_vector; 
		char* command_array = strtok(receive_buffer, " ");
		while(command_array != NULL){
			command_vector.push_back(command_array);
			command_array = strtok(NULL, " ,.-");
		}
		
		// command: start_election password
		if (strcmp(command_vector[0], "start_election") == 0){
			if(command_vector[1] != NULL){
				std::string password = command_vector[1];
				password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
  
				bool ok = true;
				
				if(default_password == password){
					ok = true;
				}else{
					ok = false;
				}

				if (ok == true){
					string str = start_election(password);
                    pthread_mutex_lock(&lock1);
					strcpy(send_buffer, str.c_str());
                    pthread_mutex_unlock(&lock1);
					
				}else{
					string str = "Wrong command or password.\n";
                    pthread_mutex_lock(&lock1);
					strcpy(send_buffer, str.c_str());
                    pthread_mutex_unlock(&lock1);
				}	
				
			}else{
				string str = "Wrong command or password.\n";
                pthread_mutex_lock(&lock1);
				strcpy(send_buffer, str.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: end_election password
		else if (strcmp(command_vector[0], "end_election") == 0){
			if(command_vector[1] != NULL){
                std::string password = command_vector[1];
			    password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());

				bool ok = true;
				
				if(default_password == password){
					ok = true;
				}else{
					ok = false;
				}
			
				if (ok == true){
					std::string result = end_election(password);
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}else{
                    std::string result = "Wrong command or password.\n";
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}	
				
			}else{
                std::string result = "Wrong command or password.\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
	    // command: add_candidate password candidate
		else if (strcmp(command_vector[0], "add_candidate") == 0){
			if(command_vector[1] != NULL && command_vector[2] != NULL){
				std::string password = command_vector[1];	
				password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());

				bool ok = true;
				
				if(default_password == password){
					ok = true;
				}else{
					ok = false;
				}
			
				if (ok == true && command_vector[2] != NULL){
					const std::string& candidate = command_vector[2];
					std::string result = add_candidate(password, candidate);
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}else if (ok == false && command_vector[2] != NULL){
					const std::string& candidate = command_vector[2];
					std::string result = add_candidate(password, candidate);
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}	
				
			}else{
                std::string result = "Wrong command or password.\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: shutdown password
		else if (strcmp(command_vector[0], "shutdown") == 0){
			if(command_vector[1] != NULL){
				std::string password = command_vector[1];
				password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
				
				bool ok = true;   
				
				if(default_password == password){
					ok = true;
				}else{
					ok = false;
				}			
				
				if (ok == true){
					std::string result = shutdown(password);
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}else{
                    std::string result = "Wrong command or password.\n";
                    pthread_mutex_lock(&lock1);
                    strcpy(send_buffer, result.c_str());
                    pthread_mutex_unlock(&lock1);
				}	
				
			}else{
                std::string result = "Wrong command or password.\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: add_voter voterid
		else if (strcmp(command_vector[0], "add_voter") == 0){
			if(command_vector[1] != NULL){
				int voterid = atoi(command_vector[1]);
				std::string result = add_voter(voterid);
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}else{
                std::string result = "Wrong command!\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: vote_for name voterid
		else if (strcmp(command_vector[0], "vote_for") == 0){
			if(command_vector[1] != NULL && command_vector[2] != NULL){
				const std::string& name = command_vector[1];
				int voterid = atoi(command_vector[2]);
				std::string result = vote_for(name, voterid);
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}else{
                std::string result = "Wrong command!\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: check_registration_status voterid
		else if (strcmp(command_vector[0], "check_registration_status") == 0){
			if(command_vector[1] != NULL){
				int voterid = atoi(command_vector[1]);
				std::string result = check_registration_status(voterid);
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}else{
                std::string result = "Wrong command!\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: check_voter_status voterid magicno
		else if (strcmp(command_vector[0], "check_voter_status") == 0){
			if(command_vector[1] != NULL && command_vector[2] != NULL){
				int voterid = atoi(command_vector[1]);
				int magicno = atoi(command_vector[2]);
				std::string result = check_voter_status(voterid, magicno);
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}else{
                std::string result = "Wrong command!\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: list_candidates
		else if (strcmp(command_vector[0], "list_candidates") == 0){
			std::string result = list_candidates();
            pthread_mutex_lock(&lock1);
            strcpy(send_buffer, result.c_str());
            pthread_mutex_unlock(&lock1);
		}		
		
		// command: vote_count name
		else if (strcmp(command_vector[0], "vote_count") == 0){
			if(command_vector[1] != NULL){
				std::string name = command_vector[1];
                name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
				std::string result = vote_count(name);
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}else{
                std::string result = "Wrong command!\n";
                pthread_mutex_lock(&lock1);
                strcpy(send_buffer, result.c_str());
                pthread_mutex_unlock(&lock1);
			}
		}
		
		// command: view_result
		else if (strcmp(command_vector[0], "view_result") == 0){
			std::string result = view_result();
            pthread_mutex_lock(&lock1);
            strcpy(send_buffer, result.c_str());
            pthread_mutex_unlock(&lock1);
		}
	
		else {
            std::string result = "Wrong command! Please try it again.";
            pthread_mutex_lock(&lock1);
            strcpy(send_buffer, result.c_str());
            pthread_mutex_unlock(&lock1);
		}

	return send_buffer;
}

void * serverThread(void *p) {
    if (shutdown_req == true){
        pthread_detach(tid);
        return 0;
    }
    
    int newSocket = *((int *)p);   
    
    pthread_mutex_lock(&lock1);
    receive_buffer[1024] = {0};
    send_buffer[1024] = {0};

    if (recv(newSocket, receive_buffer, 1024, 0) < 0){
      printf("ERROR: receiving data\n");
    }
    pthread_mutex_unlock(&lock1);
    
	function();  
        
    pthread_mutex_lock(&lock1);
    if (send(newSocket, send_buffer, 1024, 0) < 0){
      printf("ERROR: sending data\n");
    }
    
    std::string shutdown_password = "shutdown";
    std::string rec_buf = string(receive_buffer);
    int cmp = strcmp(rec_buf.c_str(), shutdown_password.c_str());
    
    if (cmp == 0 && shutdown_req){
        pthread_detach(tid);
        pthread_mutex_unlock(&lock1);
        pthread_exit(0);
        return 0;
    }

    pthread_mutex_unlock(&lock1);
    pthread_exit(0);
    return 0;
}

int serverSocket(const std::string& port){
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(stoi(port));
	
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    int sock = socket(PF_INET, SOCK_STREAM, 0);
	
    if(sock == -1) { 
      printf("Error: create\n");
      return -1; 
    }

    if (bind(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1){ 
      printf("ERROR: bind\n");
      shutdown(sock, SHUT_RDWR);
      return -1; 
    };

	if(listen(sock, 90) < 0){
      printf("ERROR: listen\n");
      shutdown(sock, SHUT_RDWR);
      return -1; 
	}
	
	while(true){
        if (shutdown_req == true) {
            pthread_detach(tid);
            break;
        }
		socklen_t addr_size = sizeof serverStorage;
		int newSock = accept(sock, (struct sockaddr *) &serverStorage, &addr_size);

        pthread_create(&tid, NULL, serverThread, &newSock);
		pthread_join(tid, NULL);
	}
  
  return 0;
}


int main(int argc, char *argv[]) { 
    
    ofstream pw_file;
    pw_file.open("password.txt");
    pw_file << "cit595" << "\n";
    pw_file.close();
    
    ofstream pt_file;
    pt_file.open("port.txt");
    pt_file << "10000" << "\n";
    pt_file.close();
    
    int o;
    const char *optstring = "ra:p:"; 
    while ((o = getopt(argc, argv, optstring)) != -1) {
        switch (o) {
            case 'r':
				{
                    std::ifstream file("backup.txt");
                    std::string output; 
                    bool readingVotingRes = false;
                    bool readingVoters = false;
                    bool readingMagicnos = false;
                    while (std::getline(file, output)){
                        const char *coutput = output.c_str();
                        std::string voting = "VOTING RESULT:";
                        const char *cvoting = voting.c_str();
                        std::string voters = "VOTERS LIST:";
                        const char *cvoters = voters.c_str();
                        std::string magicno = "MAGICNO LIST:";
                        const char *cmagicno = magicno.c_str();
                        std::string empty = "";
                        const char *cempty = empty.c_str();
					
                        if (strcmp(coutput, cvoting) == 0){
	
							readingVotingRes = true;
							readingVoters = false;
							readingMagicnos = false;
							
							continue;
							
                        }else if (strcmp(coutput, cvoters) == 0){
	
							readingVotingRes = false;
							readingVoters = true;
							readingMagicnos = false;
							
							continue;
						}else if (strcmp(coutput, cmagicno) == 0){
	
							readingVotingRes = false;
							readingVoters = false;
							readingMagicnos = true;
							
							continue;
						}else if (strcmp(coutput, cempty) == 0){
							continue;
						}
					
					if (readingVotingRes == true){
						vector<string> result;
						std::string deli = ": ";
						const char *cdeli = deli.c_str();
						char *coutput = new char[output.size()+1];
						strcpy(coutput, output.c_str());
						char *token = strtok(coutput, cdeli);
						while (token != NULL){
							result.push_back(token);
							token = strtok(NULL, " ");
						}
							
						candidate_list.push_back(result.at(0));
						int count = stoi(result.at(1));
						count_list.push_back(count);
						
					}else if (readingVoters == true){
						int voterid = stoi(output);
						voter_list.push_back(voterid);
						
					}else if (readingMagicnos == true){
						int magicno = stoi(output);
						magicno_list.push_back(magicno);
					}   
				}
				
                    file.close();
                    exists = true;
					
                    std::ifstream file_password("password.txt");
                    std::string output_password; 
                    while (std::getline(file_password, output_password)){
                        default_password = output_password;
                    }
                    file_password.close();
			
                    std::ifstream file_port("port.txt");
                    std::string output_port; 
                    while (std::getline(file_port, output_port)){
                        default_port = output_port; 
                    }
                    file_port.close();
                    
                    serverSocket(default_port);
                    break;
				}
				
            case 'a':
				{
                    ofstream file;
                    file.open("password.txt");
                    file << optarg << "\n";
                    file.close();
                    
                    default_password = optarg;
                    serverSocket(default_port);
                    break;               
				}
				
			case 'p':
				{
                    ofstream file;
                    file.open("port.txt");
                    file << optarg << "\n";
                    file.close();
                    
                    default_port = optarg;
                    serverSocket(default_port);
                    break;               
				}
			
			default:
                {
                    ofstream pfile;
                    pfile.open("port.txt");
                    pfile << "10000" << "\n";
                    pfile.close();		
                
                    ofstream pfile2;
                    pfile2.open("password.txt");
                    pfile2 << "cit595" << "\n";
                    pfile2.close();	
                    
                    default_port = "10000";
                    default_password = "cit595";
                    serverSocket(default_port);
                    break;
                }
			
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
	}
	
	if ((o = getopt(argc, argv, optstring)) == -1){
		std::ifstream file_password("password.txt");
        std::string output_password; 
        while (std::getline(file_password, output_password)){
            default_password = output_password;
        }
		file_password.close();
        
		std::ifstream file_port("port.txt");
        std::string output_port;
        while (std::getline(file_port, output_port)){
            default_port = output_port; 
        }
		file_port.close();
        
		serverSocket(default_port);
	}
	
	return 0;
}
