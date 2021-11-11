/* Network applications and network management */

#include <iostream>
#include <string>
#include <map>
#include <ActiveSocket.h>
using namespace std;

#define MAX_OPTIONS 10


map<string,string> get_options(string params){

    // var declaration
   string split[MAX_OPTIONS];
   string error = "";
   int optCount = 0;
    // default options
    map<string,string> options{
            {"r/w","false"},{"-a","127.0.0.1"},{"-c","binary"},{"-d","false"},
            {"-m","false"},{"-t","false"},{"-s","false"}};


   // split string into array
   int n = params.length();
   for(int i=0;i<n;i++){
        if (isspace(params[i])){
            optCount++;
            if (optCount >= MAX_OPTIONS){
                cerr << "Error - too many options\n";
                return {};
            } else continue;
        }else{
            split[optCount] += params[i];
        }
   }

   // parsing array into options (custom getopts)
   for (int i = 0; i <= optCount; i++) {

       // get option and argument
       string opt = split[i];
       string opt_arg = (i == optCount) ? "not_provided":split[i + 1];

       // read or write option
       if (opt == "-R") {
           if (options["r/w"] == "write") error += "cannot read and write file in one request\n";
           else options["r/w"] = "read";
       } else if (opt == "-W") {
           if (options["r/w"] == "read") error += "cannot read and write file in one request\n";
           else options["r/w"] = "write";

       // options with arguments
       } else if (opt == "-d" || opt == "-t" || opt == "-s" || opt == "-c" || opt == "-a") {
           if (opt_arg.compare("not_provided") && opt_arg[0] != '-') {
               options[opt] = split[i + 1];
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

   // check for mandatory options
   if (options["r/w"].compare("false") == 0) {
       error += "missing option -R/-W\n";
   }
   if (options["-d"].compare("false") == 0){
       error += "missing option -d [file]\n";
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

void printOptions(map<string,string> options){
    map<string,string>::iterator j;
    cout << "options: \n";
    for(j = options.begin(); j != options.end(); j++){
        cout << j->first << " : " << j->second << "\n";
    }
}

void TFTP_client(map<string, string> options){
    printOptions(options);


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
                TFTP_client(options);
            }
        }
    }
}

