

#ifndef _HTTPPARSER_H
#define _HTTPPARSER_H

typedef struct {
    string Type, path, version, connection, host;
} HttpReq_Header;

typedef struct {
    string version, status_code, status_string;
    string contType, contLength, connection;
} HttpResp_Header;

string File_Type(string& ext);
extern string Http_RespBuilder(HttpResp_Header* Resp_Header);
extern bool HttpReq_Parser(HttpReq_Header* ReqHeader, string request);

#endif 
