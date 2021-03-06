/*! \file Red.h*/ 
#ifndef RED_H_INCLUDED
#define RED_H_INCLUDED

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <curl/curl.h>
#include <fstream>
#include <sys/wait.h>

using namespace std;
/**
List d'enum
**/
enum Red_Option { 

  SET_HOST, /**< This option let the user change the hostname if he doesn't want to use our Red server */  
  SET_RED_HOST, /**< This option will make the user use our Red server (Recommended) if he didn't use the red-config option*/  
  SET_DATA_TYPE, /**< This option let the user choose the reference of the data he wants to send */ 
  SET_BUFFER, /**< This option let the user enable the buffer option that will save his datas into a variable if he wants to send it back later */ 
  SET_NEW_PERMISSION, /**< COMING SOON */ 
  GET_HOST,   /**< Return the current host whe are sending to */ 
  GET_DATA_TYPE,  /**< returns the reference of the data he wants to send */ 
  GET_BUFFER, /**< returns the buffer */ 
  LIST_PERMISSION, /**< COMING SOON */ 
  SEND_DATAS, /**< Send the data and data_type */ 
  GET_DATAS, /**< Returns the last specific data from the specified data type */ 
  ADD_NEW_DEVICE, /**< COMING SOON*/ 
  SET_PASSPHRASE, /**< Set the passphrase fot the certificate */ 
  SET_CERTIFICATE,/**< Set the path of your Red certificate */ 
  UPDATE /**< Update the software */ 
  };
/** This this the link to our Red Server */ 
static const string Red_adress ="https://device.red-cloud.io";
/** This is the class Red that will support all our code*/
class Red{  
  
  protected:
  string host;
  string body;   
  string header;
  string data_type;
  string buffer;
  string passphrase;
  string certificate;
  string token;
  long HTTPcode;
  string id_update;
  

  public:   
    Red(string ahost,string adata_type);
    Red();
    ~Red();

    virtual void display();
  
    virtual string post(Red* red);        
    virtual string get(Red* red);
    virtual string update(Red* red);
    
    virtual string set_red_option(Red* red,Red_Option option,string value);
    virtual string set_red_option(Red* red,Red_Option option,int value);
    virtual string set_red_option(Red* red,Red_Option option,float value);
    virtual string set_red_option(Red* red,Red_Option option,char value);

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and no parameter*/
    virtual string set_red_option(Red* red,Red_Option option); 

    virtual void set_certificate(string path_cert);
    virtual void set_passphrase(string new_pass);
    virtual void set_host(string new_host);
    virtual void set_id_update(string new_id);
    virtual void set_body(string new_body);
    virtual void set_buffer(string abuffer);
    virtual void set_data_type(string type);
    virtual void append_host(string append_host);
    virtual void append_body(string append_body);
    virtual void set_token(string new_token);
    virtual void set_HTTPcode(long new_http);
    
    virtual string get_certificate();
    virtual string get_passphrase();
    virtual string get_host();
    virtual string get_id_update();
    virtual string get_body();
    virtual string get_buffer();
    virtual string get_data_type();
    virtual string get_token();
    virtual long get_HTTPcode();

};
/** This function will parse the bearer header in JSON to get the token */
string parse_token(string result)
{
  int from;
  int to;
  from = result.find("token")+8;
  to = result.find("message\":\"Unauthorized : invalid token")-59;  
  return result.substr(from,to);
}
/** This function will parse the return from GET JSON object to select only the value */
string parse_get(string result)
{
  int from;
  int to;
  from = result.find("value")+8;
  to = result.find("date")-13;  
  return result.substr(from,to);
}
/** This function will parse the return JSON from update and select only the key of the software*/
string parse_id_update(string result)
{
   
  if (result.size()==2)
  {
cout<< result <<endl;
    return "NULL";
  }
  int from;
  int to;
  from = result.find("id")+5;
  to = result.find("name")-11; 
  return result.substr(from,to);
}

/*
string exec(const char* cmd) {
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}
*/
/** This function will create a red object with the red server adress by default*/
Red* red_config()
{
  Red* red_init=new Red();
  red_init->set_host(Red_adress) ;  
  red_init->set_id_update("");
  return red_init;
}
/** Red constructor*/
Red::Red(string ahost,string adata_type) :
  host(ahost),data_type(adata_type)
{}
/** Red constructor*/
Red::Red()
{}
/** Red destructor*/
Red::~Red()
{
  //cout << endl<<"Red object being destroyed!" << endl;
}

/** Display the info of the red object created*/
void Red::display(){;
  cout <<endl<< "The host is: " << host<<endl<<"The data type is: "<<data_type<<endl<<"The certificate passphrase is: "<<certificate<<endl;
  !(buffer.empty()) ? cout<<"The buffer is: "<<buffer<<endl : cout<<endl;
}

size_t handleHeader(void *ptr, size_t size, size_t count, void *stream){ 
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}

size_t handleBody(void *ptr, size_t size, size_t count, void *stream) {
  ((string*)stream)->append((char*)ptr, 0, size*count);
  return size*count;
}
/** Struct for the file creator*/
struct FtpFile {
  const char *filename;
  FILE *stream;
};

static size_t handleFile(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out=(struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */ 
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */ 
  }
  return fwrite(buffer, size, nmemb, out->stream);
}
string Red::post (Red* red)
{
  CURL *curl;
  CURLcode res;
  struct curl_slist *headers = NULL;
  long response_HTTP_CODE_from_server;  
  string response_POST_from_server;
  string response_header;  
  string token_header= "authorization: bearer "+red->get_token();
  string host=red->get_host();
  string post=red->get_body();
  string cert=red->get_certificate();
  string passphrase = red->get_passphrase();

  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, host.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, token_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str());

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    //Add the certificate    
    curl_easy_setopt(curl,CURLOPT_SSLCERT,cert.c_str());

    //Add the passphrase
    curl_easy_setopt(curl,CURLOPT_KEYPASSWD,passphrase.c_str());
    /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
       itself */
    //curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 4);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleBody);  
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,&response_POST_from_server);    
   
    curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, handleHeader );
    curl_easy_setopt( curl, CURLOPT_HEADERDATA, &response_header );

    /* Perform the request, res will get the return code */
  
    res = curl_easy_perform(curl);   

    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_HTTP_CODE_from_server);
    red->set_HTTPcode(response_HTTP_CODE_from_server);
    //cout <<"HTTPS CODE :" + to_string(response_HTTP_CODE_from_server)<<endl ;
    //cout <<"HEADER: " + response_header <<endl ;
    //cout <<"BODY: " +response_POST_from_server <<endl;

    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);  
  }
  
  if( response_HTTP_CODE_from_server==401)
  {   
    red->set_token(parse_token(response_POST_from_server));   
    return to_string(red->get_HTTPcode());
  }
 
    return to_string(red->get_HTTPcode());
 }
string Red::get (Red* red)
{
  CURL *curl;
  CURLcode res;
  string response_GET_from_server; 
  struct curl_slist *headers = NULL;
  string host = red->get_host();
  string cert = red->get_certificate();
  string passphrase = red->get_passphrase();
  long response_HTTP_CODE_from_server;
  string response_header;  
  string token_header= "authorization: bearer "+red->get_token();
  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, host.c_str());
    headers = curl_slist_append(headers, token_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
     //Add the certificate    
    curl_easy_setopt(curl,CURLOPT_SSLCERT,cert.c_str());

    //Add the passphrase
    curl_easy_setopt(curl,CURLOPT_KEYPASSWD,passphrase.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleBody);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_GET_from_server);
    
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_HTTP_CODE_from_server);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));    

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
   
  if( response_HTTP_CODE_from_server==401)
  {   
    red->set_token(parse_token(response_GET_from_server));   
    return red->get(red);
  } 
      return parse_get(response_GET_from_server) ;
  }

string Red::update (Red* red)
{
  CURL *curl;
  CURLcode res;
  string response_GET_from_server; 
  struct curl_slist *headers = NULL;
  string host = red->get_host();
  string cert = red->get_certificate();
  string passphrase = red->get_passphrase();
  long response_HTTP_CODE_from_server;
  string response_header;  
  string token_header= "authorization: bearer "+red->get_token();
  curl_global_init(CURL_GLOBAL_DEFAULT);

 struct FtpFile ftpfile={
    "RED-Update/update", /* name to store the file as if successful */ 
    NULL
    };  
  curl = curl_easy_init();
  if(curl) {

    curl_easy_setopt(curl, CURLOPT_URL, host.c_str());
    headers = curl_slist_append(headers, token_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
     //Add the certificate    
    curl_easy_setopt(curl,CURLOPT_SSLCERT,cert.c_str());

    //Add the passphrase
    curl_easy_setopt(curl,CURLOPT_KEYPASSWD,passphrase.c_str());
    if(red->get_id_update()=="")
    {
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleBody);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_GET_from_server);
    }else {
     
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handleFile);    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile); 
    }

    
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &response_HTTP_CODE_from_server);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));    

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  
  if( response_HTTP_CODE_from_server==401)
  {   
    red->set_token(parse_token(response_GET_from_server)); 
    return red->update(red);
  } 
  
  if(red->get_id_update()=="")
  {        

      red->set_id_update(parse_id_update(response_GET_from_server));    

      if(red->get_id_update()=="NULL")
      {
        return 0; 
      }
      red->append_host(red->get_id_update());
      return red->update(red);
  }
  if(ftpfile.stream)
    fclose(ftpfile.stream); /* close the local file */ 
      cout<<response_GET_from_server<<endl;
      return to_string(response_HTTP_CODE_from_server);
  }

void Red::set_host(string new_host)
{
  host=new_host;
}
void Red::set_id_update(string new_id)
{
  id_update=new_id;
}

void Red::set_body(string new_body)
{
  body=new_body;
}

void Red::set_data_type(string type)
{
  data_type=type;
}
void Red::set_passphrase(string new_pass)
{
  passphrase=new_pass;
}
void Red::set_certificate(string path_cert)
{
  certificate=path_cert;
}
void Red::set_token(string new_token)
{
  ofstream cache;
  cache.open ("cache.txt"); 
  cache << new_token;
  cache.close();
  token=new_token;
}

void Red::set_HTTPcode(long new_http)
{
  HTTPcode=new_http;
}

string Red::get_host()
{
  return host;
}
string Red::get_id_update()
{
  return id_update;
}

string Red::get_body()
{
  return body;
}
string Red::get_token()
{   
  ifstream cache;
  cache.open ("cache.txt");
  string output; 
  getline(cache,output); 
  cache.close();
  return output;
}
void Red::append_body(string append_body)
{
  body+=append_body;  
}
void Red::append_host(string append_host)
{
  host+=append_host;
 
}
string Red::get_data_type()
{ 
  return data_type;
}
void Red::set_buffer(string abuffer)
{
    buffer=abuffer;
}
string Red::get_buffer()
{
    return buffer;
}
string Red::get_passphrase()
{ 
  return passphrase;
}
string Red::get_certificate()
{ 
  return certificate;
}
long Red::get_HTTPcode()
{ 
  return HTTPcode;
}

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and a string has a parameter*/
string Red::set_red_option(Red* red,Red_Option option,string value)
{    
  string code ;
    switch (option) 
    {
        case SEND_DATAS :
          red->set_body("{\"datatype\":\"");
          red->append_body(red->get_data_type());
          red->append_body("\",\"value\":\"");
          red->append_body(value);
          red->append_body("\"}");  
          red->get_host() == Red_adress ? red->append_host("/device/newdata") : red->append_host(""); 
          
          if(red->post(red)=="401")
          {    
            cout<<"Old token.. getting a new one\n";       
            return red->post(red);
          }else {
            return to_string(red->get_HTTPcode()); 
          }      
        break;        
        case SET_PASSPHRASE:        
            red->set_passphrase(value);
        break;
        case SET_CERTIFICATE:        
            red->set_certificate(value);
        break;
        case SET_DATA_TYPE :
            red->set_data_type(value);
        break;
        case SET_HOST :
            red->set_host(value);
        break;
        case SET_BUFFER :
          red->set_buffer(value);
        break;
        default:
        cout<< "not recognized option: "<< value;
    }
    return "OK";
}

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and a int has a parameter*/
string Red::set_red_option(Red *red,Red_Option option,int value)
{
switch (option)
{
   case SEND_DATAS :
          red->set_body("{\"datatype\":\"");
          red->append_body(red->get_data_type());
          red->append_body("\",\"value\":\"");
          red->append_body(to_string(value));
          red->append_body("\"}");      
          red->get_host() == Red_adress ? red->append_host("/device/newdata") : red->append_host(""); 
          red->post(red);
          if(red->get_HTTPcode()==401)
          {    
            cout<<"Old token.. getting a new one\n";       
            return red->post(red);
          }else {
            return to_string(red->get_HTTPcode()); 
          }      
          break;  
    case ADD_NEW_DEVICE:          
          red->get_host()== Red_adress ? red->append_host("/device/new/") : red->append_host("");    
          red->append_host(to_string(value));      
          return red->get(red);
      break;
   
   default:
    return "not recognized option";
}      
       return "OK";    
}

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and a float has a parameter*/
string Red::set_red_option(Red* red,Red_Option option, float value)
{   
  switch (option)
  { 
   case SEND_DATAS :
          red->set_body("{\"datatype\":\"");
          red->append_body(red->get_data_type());
          red->append_body("\",\"value\":\"");
          red->append_body(to_string(value));
          red->append_body("\"}");          
          red->get_host() == Red_adress ? red->append_host("/device/newdata") : red->append_host(""); 
          red->post(red);
          if(red->get_HTTPcode()==401)
          {    
            cout<<"Old token.. getting a new one\n";       
            return red->post(red);
          }else {
            return to_string(red->get_HTTPcode()); 
          }      
   break;
   default:
    return "not recognized option";
  }    
    return "OK";
}

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and a char has a parameter*/
string Red::set_red_option(Red* red,Red_Option option, char value)
{     
  switch (option)
  {  
   case SEND_DATAS :         
          red->set_body("{\"datatype\":\"");
          red->append_body(red->get_data_type());
          red->append_body("\",\"value\":\"");
          red->append_body(to_string(value));
          red->append_body("\"}");         
          red->get_host() == Red_adress ? red->append_host("/device/newdata") : red->append_host(""); 
          red->post(red);
          if(red->get_HTTPcode()==401)
          {    
            cout<<"Old token.. getting a new one\n";       
            return red->post(red);
          }else {
            return to_string(red->get_HTTPcode()); 
          }      
   break;   
   default:
    return "not recognized option";
  }    
    return "OK";
}

/**The main method the user will have to use to communicate with he API Here you can send a Red_Option and no parameter*/
string Red::set_red_option(Red* red,Red_Option option)
{
  string res;  
  int ret;
  ofstream update_file;
    switch(option)
    {
      case  GET_HOST:
      return red->get_host();
      break;
      case  GET_DATA_TYPE:
      return red->get_data_type();
      break;
      case  GET_BUFFER:
      return red->get_buffer();
      break;
      case  GET_DATAS:
      red->append_host("/device/");
      red->append_host(red->get_data_type());        
      return red->get(red);
      break;
      case ADD_NEW_DEVICE:          
          red->get_host()== Red_adress ? red->append_host("/device/new/1") : red->append_host("");          
          return red->get(red);
      break;
      case SET_RED_HOST:
             red->set_host(Red_adress);
      return "Red servers are now the new host";
      case LIST_PERMISSION:
      return "INCOMMING";
      case UPDATE:           
      red->append_host("/device/update/");
      res = red->update(red);      
      if(res=="200")
      {      
        ret=system("sudo dpkg -i RED-Update/update");
      if (WEXITSTATUS(ret) == 0x0)
      {
      //Bien fini et bien executé
        red->set_host(Red_adress) ;  
        red->append_host("/device/update/ack/");
        red->append_host(red->get_id_update());
        update_file.open ("RED-Update/update_version.txt"); 
        update_file << red->get_id_update();
        update_file.close();       
        if(red->post(red)=="401")
          {    
            cout<<"Old token.. getting a new one\n";       
            return red->post(red);
          }else {            
            return to_string(red->get_HTTPcode()); 
          }      

     }else
      return "Erreur instalation .deb";
      }else 
      {
        return "Exit code :"+res ;
      }
     
      default:
      return "no recognize option: ";
    }
    
}
#endif // RED_H_INCLUDED