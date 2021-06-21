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

#include "Election.h"

using namespace std;

pthread_mutex_t lock1;
pthread_t tid;
vector<char*> command_vector;
std::string default_password = "cit595";

std::vector<std::string> candidate_list;
std::vector<int> count_list;  
std::vector<int> voter_list;
std::vector<int> magicno_list;

bool exists = false;
     
std::string start_election(const std::string& password){
    pthread_mutex_lock(&lock1);
    
    if(exists == false){
        exists = true;
        
        candidate_list.clear();
        count_list.clear();  
        voter_list.clear();
        magicno_list.clear();
        
        pthread_mutex_unlock(&lock1);
        cout << "OK" << endl;
		return "OK";
	}else{
        pthread_mutex_unlock(&lock1);
        cout << "EXISTS" << endl;
		return "EXISTS";
		}
    
	pthread_mutex_unlock(&lock1);
    cout << "ERROR" << endl;
	return "ERROR"; 	
}

std::string add_candidate(const std::string& password, std::string candidate){
    pthread_mutex_lock(&lock1);
    
    if (exists == false){
        pthread_mutex_unlock(&lock1);
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    candidate.erase(std::remove(candidate.begin(), candidate.end(), '\n'), candidate.end());
    
    if (std::find(candidate_list.begin(), candidate_list.end(), candidate) != candidate_list.end()){
        pthread_mutex_unlock(&lock1);
        cout << "EXISTS" << endl;
        return "EXISTS";
    }else{
        
        candidate_list.push_back(candidate);
        count_list.push_back(0);
        pthread_mutex_unlock(&lock1);
        cout << "OK" << endl;
        return "OK";
    }
}

std::string shutdown(const std::string& password){
    if (exists == false){
        cout << "ERROR" << endl;
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
 
    std::string default_password = "cit595";
    std::string default_port = "10000";
    
    pthread_mutex_unlock(&lock1);
    
    cout << "OK" << endl;
	return "OK";
}

std::string add_voter(int voterid){
    if (exists == false){
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    if (voterid <= 1000 || voterid >= 9999){
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    if (std::find(voter_list.begin(), voter_list.end(), voterid) != voter_list.end()){
        cout << "EXISTS" << endl;
        return "EXISTS";
    }else{
        pthread_mutex_lock(&lock1);
        voter_list.push_back(voterid);
        pthread_mutex_unlock(&lock1);
        cout << "OK" << endl;
        return "OK";
    }
}

std::string check_registration_status(int voterid){
    if (exists == false){
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    if (voterid<1000 || voterid>9999){
        cout << "INVALID" << endl;
        return "INVALID";
    }else if (std::find(voter_list.begin(), voter_list.end(), voterid) != voter_list.end()){
        cout << "EXISTS" << endl;
        return "EXISTS";
    }else{
        cout << "UNREGISTERED" << endl;
        return "UNREGISTERED";
    }
}

std::string check_voter_status(int voterid, int magicno){
    if (exists == false){
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    if (std::find(voter_list.begin(), voter_list.end(), voterid) == voter_list.end()){
        cout << "CHECKSTATUS" << endl;
        return "CHECKSTATUS";
    }else{
        if (std::find(magicno_list.begin(), magicno_list.end(), magicno) != magicno_list.end()){
            cout << "ALREADYVOTED" << endl;
            return "ALREADYVOTED";
        }else{
            cout << "UNAUTHORIZED" << endl;
            return "UNAUTHORIZED";
        }
    }
}

std::string vote_for(std::string name, int voterid){
    if (exists == false){
        cout << "ERROR" << endl;
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
            cout << result << endl;
            return result;
        }else{	
            cout << "ERROR" << endl;
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
        cout << result << endl;
        return result;
    }
}

std::string list_candidates(){
    if (exists == false){
        cout << "ERROR" << endl;
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
    cout << res << endl;
    return res;
}

std::string vote_count(std::string name){
    if (exists == false){
        cout << "-1" << endl;
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
    cout << count << endl;
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
        for (unsigned int i = 0; i < count_list.size()-1; i++) {
            res += candidate_list[winner_list[i]];
            res += ">,<";
        }
        res += candidate_list[winner_list[winner_list.size()-1]];
        res += ">";
    }
    
    pthread_mutex_unlock(&lock1);
    cout << res << endl;
	return res;
}

std::string end_election(const std::string& password){
    if (exists == false){
        cout << "ERROR" << endl;
        return "ERROR";
    }
    
    std::string res = view_result();
	exists = false;
	return res;
}

void* function(void* i){
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
					start_election(password);
				}else{
					std::cout << "Wrong command or password.\n" << std::endl;	
				}	
				
			}else{
				std::cout << "Wrong command or password.\n" << std::endl;	
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
					end_election(password);
				}else{
					std::cout << "Wrong command or password.\n" << std::endl;	
				}	
				
			}else{
				std::cout << "Wrong command or password.\n" << std::endl;	
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
					add_candidate(password, candidate);
				}else if (ok == false && command_vector[2] != NULL){
					const std::string& candidate = command_vector[2];
					add_candidate(password, candidate);
// 					std::cout << "Wrong command or password!!!!\n" << std::endl;	
				}	
				
			}else{
				std::cout << "Wrong command or password.\n" << std::endl;	
			}
		}
		
		// command: shutdown password
		else if (strcmp(command_vector[0], "shutdown") == 0){
			if(command_vector[1] != NULL){
				 std::string password = command_vector[1];
				bool ok = true;   
				    password.erase(std::remove(password.begin(), password.end(), '\n'), password.end());
				
				if(default_password == password){
					ok = true;
				}else{
					ok = false;
				}			
				
				if (ok == true){
					shutdown(password);
				}else{
					std::cout << "Wrong command or password.\n" << std::endl;	
				}	
				
			}else{
				std::cout << "Wrong command or password.\n" << std::endl;	
			}
		}
		
		// command: add_voter voterid
		else if (strcmp(command_vector[0], "add_voter") == 0){
			if(command_vector[1] != NULL){
				int voterid = atoi(command_vector[1]);
				add_voter(voterid);
			}else{
				std::cout << "Wrong command!\n" << std::endl;
			}
		}
		
		// command: vote_for name voterid
		else if (strcmp(command_vector[0], "vote_for") == 0){
			if(command_vector[1] != NULL && command_vector[2] != NULL){
				const std::string& name = command_vector[1];
				int voterid = atoi(command_vector[2]);
				vote_for(name, voterid);
			}else{
				std::cout << "Wrong command!\n" << std::endl;
			}
		}
		
		// command: check_registration_status voterid
		else if (strcmp(command_vector[0], "check_registration_status") == 0){
			if(command_vector[1] != NULL){
				int voterid = atoi(command_vector[1]);
				check_registration_status(voterid);
			}else{
				std::cout << "Wrong command!\n" << std::endl;
			}
		}
		
		// command: check_voter_status voterid magicno
		else if (strcmp(command_vector[0], "check_voter_status") == 0){
			if(command_vector[1] != NULL && command_vector[2] != NULL){
				int voterid = atoi(command_vector[1]);
				int magicno = atoi(command_vector[2]);
				check_voter_status(voterid, magicno);
			}else{
				std::cout << "Wrong command!\n" << std::endl;
			}
		}
		
		// command: list_candidates
		else if (strcmp(command_vector[0], "list_candidates\n") == 0){
			list_candidates();
		}		
		
		// command: vote_count name
		else if (strcmp(command_vector[0], "vote_count\n") == 0){
			if(command_vector[1] != NULL){
				const std::string& name = command_vector[1];
				vote_count(name);
			}else{
				std::cout << "Wrong command!\n" << std::endl;
			}
		}
		
		// command: view_result
		else if (strcmp(command_vector[0], "view_result\n") == 0){
			view_result();
		}
	
		else {
			std::cout << "Wrong command! Please try it again." << std::endl;
		}


	return NULL;
}

char* replaceWord(const char* s, const char* oldW,
                  const char* newW)
{
    char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
  
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
  
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
  
    // Making new string of enough length
    result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
  
    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
  
    result[i] = '\0';
    return result;
}


int main(int argc, char *argv[]) {
    // TODO: Implement server-api code here for milestone 1
	
    int o;
    const char *optstring = "ra:";
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
							//std::cout<<"read :VOTING"<<std::endl;
	
							readingVotingRes = true;
							readingVoters = false;
							readingMagicnos = false;
							
							continue;
							
					}else if (strcmp(coutput, cvoters) == 0){
							//std::cout<<"read :VOTERS"<<std::endl;
	
							readingVotingRes = false;
							readingVoters = true;
							readingMagicnos = false;
							
							continue;
						}else if (strcmp(coutput, cmagicno) == 0){
							//std::cout<<"read : MAGICNO"<<std::endl;
	
							readingVotingRes = false;
							readingVoters = false;
							readingMagicnos = true;
							
							continue;
						}else if (strcmp(coutput, cempty) == 0){
							//std::cout<<"read : empty line"<<std::endl;
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
						//std::cout << "candidate list:" << result.at(0) << std::endl;
	
						count_list.push_back(count);
						//std::cout << "count_list:" << count_list[0] << std::endl;
					}else if (readingVoters == true){
						int voterid = stoi(output);
						voter_list.push_back(voterid);
						//std::cout << "voter_list:" << voterid << std::endl;
					}else if (readingMagicnos == true){
						int magicno = stoi(output);
						magicno_list.push_back(magicno);
						//std::cout << "magicno_list:" << magicno << std::endl;
					}   
				}
				
				file.close();
				exists = true;
					
				std::cout<<"The election has been continued."<<std::endl;			
				//while(true){
				//function();
				break;
				}
			//}			
            case 'a':
				{
				ofstream file;
				file.open("password.txt");
				file << optarg << "\n";
				file.close();
				std::cout<<"The password has been changed to "<< optarg <<std::endl;			
				break;               
				}
			
			default:
				//function();
				break;
			
            case '?':
                printf("error optopt: %c\n", optopt);
                printf("error opterr: %d\n", opterr);
                break;
        }
	}
	if ((o = getopt(argc, argv, optstring)) == -1){
		std::ifstream file("password.txt");
        std::string output; 
        while (std::getline(file, output)){
            default_password = output;
        }		
		file.close();
	}
    while (true){
        command_vector.clear();
        
        // get the input
		char command[100];
        fgets(command, 101, stdin); 
		char* command_array = strtok(command," ");
		while(command_array != NULL){
			command_vector.push_back(command_array);
			command_array = strtok(NULL, " ,.-");
		}
        
        if (command_vector.size() != 0){
            int i=0;
            pthread_create(&tid, NULL, function, &i);
            pthread_join(tid, NULL);
        }
        
        std::string shutdown_password = default_password+"\n";
        if (strcmp(command_vector[0], "shutdown") == 0){
            if (strcmp(command_vector[1], shutdown_password.c_str()) == 0){
                pthread_detach(tid);
                break;
            }
        }
    }
	
	return 0;
}