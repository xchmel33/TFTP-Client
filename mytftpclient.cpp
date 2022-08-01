/**************************************************/
/** Network applications and network management  **/
/**                                              **/
/**           Author: Lukas Chmelo               **/
/**                xchmel33                      **/
/**************************************************/


#include <iostream>
#include <string>
#include <map>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <vector>

using namespace std;

#define OP_RRQ 1
#define OP_WRQ 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5
#define BUFFER_SIZE 516

/** Php like function to split string into array based on character inside string */
vector<string> split(string s, char c){
    int n = s.length();
    int split_count = 0;
    vector<string> output(count(s.begin(),s.end(),c)+1);
    for (int i = 0; i < n; ++i) {
        if(s[i] == c){
            split_count++;
            continue;
        }
        else{
            output[split_count] += s[i];
        }
    }
    return output;
}

/** Argument parser into program options */
map<string,string> get_options(string params){

    string error = "";

    // split params string to array
    vector<string> params_array;
    params_array = split(params,' ');
    int optCount = (int) params_array.size();

    // default options
    map<string,string> options{
            {"r/w","false"},{"-a","127.0.0.1,69"},{"-c","binary"},{"-d","false"},
            {"-m","false"},{"-t","0"},{"-s","512"}};

    // parsing array into options (custom getopts)
    for (int i = 0; i < optCount; i++) {

        // get option and argument
        string opt = params_array[i];
        string opt_arg = (i+1 == optCount) ? "not_provided":params_array[i + 1];

        // read or write option
        if (opt == "-R") {
            if (options["r/w"] == "write") error += "cannot read and write file in one request\n";
            else options["r/w"] = "READ";
        } else if (opt == "-W") {
            if (options["r/w"] == "read") error += "cannot read and write file in one request\n";
            else options["r/w"] = "WRITE";

        // options with arguments
        } else if (opt == "-d" || opt == "-t" || opt == "-s" || opt == "-c" || opt == "-a") {
            if (opt_arg.compare("not_provided") && opt_arg[0] != '-') {
                options[opt] = params_array[i + 1];
                i++;
            } else {
                error += " option " + opt + " requires an argument\n";
            }

            // options without arguments
        } else if (opt == "-m") {
            options[opt] = "true";
        }
        else{
            error += "invalid option "+opt+"\n";
        }
    }

    //timeout & size string denied
    char* p_size;
    char* p_timeout;
    strtol(options["-s"].c_str(),&p_size,10);
    strtol(options["-t"].c_str(),&p_timeout,10);
    if (*p_size){
        error += "invalid size ";
        error += options["-s"];
        error += "\n";
    }
    if (*p_timeout){
        error += "invalid timeout";
        error += options["-t"];
        error += "\n";
    }

    // check for mandatory options
    if (options["r/w"].compare("false") == 0) {
        error += "missing option -R/-W\n";
    }
    if (options["-d"].compare("false") == 0){
        error += "missing option -d [file]\n";
    }

    // acceptable modes
    vector<string> modes = {"ascii","netascii","octec","binary"};
    if (find(modes.begin(),modes.end(),options["-c"]) == modes.end()){
        error += "unknown mode ";
        error += options["-c"].c_str();
    }

    // final error check
    if (error.compare("") == 0){
        return options;
    }
    else{
        error = error.substr(0,error.length()-1);
        cerr << "Errors:\n" << error << "\n";
        return {};
    }
}

/** debug function */
void printOptions(map<string,string> options){
    map<string,string>::iterator j;
    cout << "options: \n";
    for(j = options.begin(); j != options.end(); j++){
        cout << j->first << " : " << j->second << "\n";
    }
    cout << endl;
}

/** get(guess) ip address format */
bool isIPV4(string address) {
    size_t n = count(address.begin(),address.end(),'.');
    return (n!=0);
}

/** file in dir */
string basename(string dir){
	vector<string> s_array = split(dir,'/');
    return (s_array.size() == 0) ? s_array[s_array.size()] : dir;
}

/** main application */
int TFTP_client(map<string, string> options) {
    /*DEBUG printOptions(options);*/

    // program options
    bool read_write = (strcmp(options["r/w"].c_str(),"READ") == 0);
    string file_modes = (read_write) ? "wb":"rb";

    // IPV4s for testing
    if (options["-a"] == "virtual") options["-a"] = "192.168.56.1,69";
    if (options["-a"] == "localhost") options["-a"] = "127.0.0.1,69";

    // parse port
    vector<string> tmp = split(options["-a"],',');
	if (tmp.size() == 0 || tmp.size() == 1){
		cerr << "Error - invalid ip!";
		return 0;
	}
    const char* address = tmp[0].c_str();
    int port = stoi(tmp[1]);

    // file
    FILE *f;
    const char* filename = basename(options["-d"].c_str());


    // variables for transfer
    int sock, counter, server_length;
    uint16_t op,block;
    struct sockaddr_in server{};
    struct hostent *host;
    char buffer[BUFFER_SIZE], *p;
    string error = "";


    // ipv4 or ipv6
    if (isIPV4(address)){
        sock = socket(AF_INET,SOCK_DGRAM,0);
        server.sin_family = AF_INET;
    } else{
        sock = socket(AF_INET6,SOCK_DGRAM,0);
        server.sin_family = AF_INET6;
    }
    if (sock == -1){
        cerr << "Error - unable to create socket, check IPV4/V6 format\n";
        return 0;
    }

    // get pointer to servers address
    host = gethostbyname(address);
    if (host == nullptr){
        cerr << "Error - unknown host " << address << endl;
        return 0;
    }

    // bind client address with server
    memcpy(&server.sin_addr,host->h_addr,host->h_length);
    server.sin_port = htons(port);


    // TFTP request packet (R/W)
    memset(buffer,0,BUFFER_SIZE);
    *(short*)buffer = read_write ? htons(OP_RRQ) : htons(OP_WRQ);
    p = buffer+2;
    strcpy(p,options["-d"].c_str());
    p += strlen(options["-d"].c_str())+1;
    strcpy(p,options["-c"].c_str());
    p += strlen(options["-c"].c_str())+1;


    // send request packet to server
    cout << "Requesting " << options["r/w"].c_str() << "from server " << address << ":" << port << endl;
    sendto(sock, buffer, p-buffer, 0, (struct sockaddr *) &server, sizeof(struct sockaddr_in));


    // open file for receiving/sending data
    f = fopen(filename, file_modes.c_str());
    if (f == nullptr) {
        cerr << "Error - unable to open " << filename << endl;
        return 0;
    }

    // download file from server (RRQ)
    if (read_write) {

        // data packet loop until short packet, which is indicating end of file
        do {

            // receive data packet
            server_length = sizeof server;
            counter = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server,
                               reinterpret_cast<socklen_t *>(&server_length));

            // error
            if (ntohs(*(short *) buffer) == OP_ERROR) {
                error += (buffer+4);
                error += "\n";
                break;
            }

            // save data packet and send acknowledgement
            else {
                cout << "Receiving DATA " << counter-4 << "bytes" << endl;

                // write to file
                fwrite(buffer + 4, 1, counter - 4, f);

                // send ack
                *(short *) buffer = htons(OP_ACK);
                sendto(sock, buffer, 4, 0, (struct sockaddr *)&server, sizeof server);
            }
        } while (counter == 516); // data size 512 bytes
    }

    // upload file to server (WRQ)
    else{
        int blockN = 0;
        do{
            // receive acknowledgement packet
            server_length = sizeof server;
            *(short *) buffer = htons(OP_ACK);
            recvfrom(sock, buffer, 4, 0, (struct sockaddr *) &server,
                               reinterpret_cast<socklen_t *>(&server_length));

            // error packet, show message
            if (ntohs(*(short * ) buffer) == OP_ERROR){
                error += (buffer + 4);
                error += "\n";
                break;
            }

            // send data packet
            else if (ntohs(*(short * ) buffer) == OP_ACK){

                // opcode & block number
                op = htons(OP_DATA);
                block = htons(blockN);
                memcpy(buffer+0,&op,2);
                memcpy(buffer+2,&block,2);

                // read from file
                size_t read_bytes = fread(buffer+4,1,512,f);
                if (read_bytes != 0) {
                    cout << "Sending DATA " << read_bytes << " bytes" << endl;

                    // read_bytes + 4 bytes for opcode and block number
                    counter = sendto(sock, buffer, read_bytes+4, 0, (struct sockaddr *) &server, sizeof(struct sockaddr_in));
                }
            }
            blockN++;
        } while (counter == 516); // data size = 512 bytes
    }
    fclose(f);
    if (error == ""){
        return 1;
    } else{
        cerr << "Errors: " << error << endl;
        if (read_write) remove(options["-d"].c_str());
        return 0;
    }
}

int main() {
    while (true) {
        // command generator
        putchar('>');

        // get and check arguments
        string params;
        getline(cin, params);
        if (params.compare("exit") == 0){
            break;
        }
        else if (params.compare("") != 0) {
            map<string,string> options = get_options(params);
            if (options.empty()){
                continue;
            }
            else{
                if (TFTP_client(options)){
                    cout << "Transfer completed without errors\n";
                }
                else{
                    cout << "Transfer failed!\n";
                }
                continue;
            }
        }
    }

}
