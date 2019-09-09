
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <regex>
#include <unordered_map>

using namespace std;

#include "HTTPParser.h"

string Http_RespBuilder(HttpResp_Header* Resp_Header){
    string resp;
    char buffer[80];
    time_t now = time(0);
    tm* ltime = gmtime(&now);
    strftime(buffer,sizeof(buffer),"%a, %d %b %Y %T GMT",ltime);
    resp = Resp_Header->version + " " + Resp_Header->status_code + " " + Resp_Header->status_string + "\r\n";
    resp += "Date: " + string(buffer) + "\r\n";
    resp += "Content-Type: " + Resp_Header->contType + "\r\n";
    resp += "Content-Length: " + Resp_Header->contLength + "\r\n";
    resp += "Connection: " + Resp_Header->connection + "\r\n";
    //resp += "Connection: " + Resp_Header->contLength + "\r\n";
    resp += "\r\n";
    return resp;
}

bool HttpReq_Parser(HttpReq_Header* ReqHeader, string request){
    //cout << "im in parser" << endl;
    string req = request;
    string Tag_Search = "";
    string Tag = "";
    smatch sm;
    regex rg("(GET) (.+) (HTTP/.+)\r\n"+ Tag +"(?:.|\n)*");
    if (regex_search(req, sm, rg))
    {
        ReqHeader->Type = sm[1];
        ReqHeader->path = sm[2];
        ReqHeader->version = sm[3];
        
        Tag = "(?:(?:.|\r|\n)*Connection:)(?:\\s*\t*)(.+)\r\n";
        rg = "(GET) (.+) (HTTP/.+)\r\n"+ Tag +"(?:.|\n)*";
        if (regex_search(req, sm, rg))
            ReqHeader->connection = sm[4];
        else
            ReqHeader->connection = "close";

        Tag = "(?:(?:.|\r|\n)*Host:)(?:\\s*\t*)(.+)\r\n";
        rg = "(GET) (.+) (HTTP/.+)\r\n"+ Tag +"(?:.|\n)*";
        if (regex_search(req, sm, rg))
            ReqHeader->host = sm[4];
        else
            ReqHeader->host = "dummy";

        return true;
        
    }
    else {
        return false;
    }
}

static unordered_map<string, string> mime_dict = {
    {"jpg", "image/jpg"},
    {"png", "image/png"},
    {"gif", "image/gif"},
    {"html", "text/html"},
    {"htm",  "text/html"},
    {"pdf",  "application/pdf"},
};

string File_Type(string& ext) {
    if (mime_dict.find(ext) != mime_dict.end()) {
        return mime_dict[ext];
    }
    return "application/octet-stream";
}
