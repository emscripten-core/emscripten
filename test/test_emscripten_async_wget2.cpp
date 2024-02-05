#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>


class http {
public:
  enum Status {
    ST_PENDING = 0,
    ST_FAILED,
    ST_OK,
    ST_ABORTED,
  };

  enum RequestType {
    REQUEST_GET = 0,
    REQUEST_POST ,
  };

  enum AssyncMode {
    ASSYNC_THREAD
  };

  // Callback
  static void onLoaded(unsigned handle, void* parent, const char * file) {
    http* req = reinterpret_cast<http*>(parent);
    req->onLoaded(file);
  }

  static void onError(unsigned handle, void* parent, int statuserror) {
    http* req = reinterpret_cast<http*>(parent);
    req->onError(statuserror);
  }

  static void onProgress(unsigned handle, void* parent, int progress) {
    http* req = reinterpret_cast<http*>(parent);
    req->onProgress(progress);
  }

  // Constructeur
  http(const char* hostname, int requestType, const char* targetFilename = "")
    : _hostname(hostname), _targetFileName(targetFilename), _request((RequestType)requestType),
      _status(ST_PENDING), _assync(ASSYNC_THREAD), _uid(uid++) {}

  /**
   * Effectue la requete
   */
  void runRequest(const char* page, int assync);

  /**
   * Abort the request
   */
  void abortRequest() {
    emscripten_async_wget2_abort(_handle);
    _status = ST_ABORTED;
  }

  /**
   * Accede a la reponse
   */
  const char* getContent() {
    return _content.c_str();
  }


  /**
   * Accede a l'erreur
   */
  const char* getError() {
    return _error.c_str();
  }

  /**
   * Accede au status
   */
  int getStatus() {
    return _status;
  }

  /**
   * Accede a la progression
   */
  int getProgress() {
    return _progressValue;
  }

  /**
   * Get Id of http Class
   */
  int getId() {
    return _uid;
  }

  /**
   *
   */
  void addValue(const char* key, const char* value);

  /**
   * Callback
   */
  void onProgress(int progress) {
    _progressValue = progress;
  }
  void onLoaded(const char* file);
  void onError(int error);

  // Static parameter
  static int uid;
  static std::string cross_domain ;

private:

  // Id of request
  int     _uid;

  // nom de l'hote
  std::string _hostname;

  // nom de la page
  std::string _page;

  // target filename
  std::string _targetFileName;

  // param
  std::string _param;

  // resultat
  std::string _content;

  // probleme
  std::string _error;

  // request type
  RequestType _request;

  // status
  int         _status;

  // progress value
  int         _progressValue = -1;

  // mode assyncrone courant
  AssyncMode  _assync;

  // request handle
  unsigned _handle;

};

//this is safe and convenient but not exactly efficient
inline std::string format(const char* fmt, ...){
  int size = 512;
  char* buffer = 0;
  buffer = new char[size];
  va_list vl;
  va_start(vl,fmt);
  int nsize = vsnprintf(buffer, size, fmt, vl);
  if (size <= nsize) {//fail delete buffer and try again
    delete buffer; buffer = 0;
    buffer = new char[nsize+1];//+1 for /0
    nsize = vsnprintf(buffer, size, fmt, vl);
  }
  std::string ret(buffer);
  va_end(vl);
  delete buffer;
  return ret;
}

int http::uid = 0;

/*
- Useful for download an url on other domain
<?php
header("Access-Control-Allow-Origin: *");
// verifie si on a les bons parametres
if( isset($_GET['url']) ) {

  $fileName = $_GET['url'];
  if($f = fopen($fileName,'rb') ){
    $fSize = 0;
    while(!feof($f)){
      ++$fSize;
      $data = fread($f,1);
    }
    fclose($f);
    if( $fSize < 1 ) {
      header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
      echo 'For empty file ' . $fileName;
      die();
    } else {
      header("POST ".$fileName." HTTP/1.1\r\n");
      header('Content-Description: File Transfer');
      header('Content-Transfer-Encoding: binary');
      header('Content-Disposition: attachment; filename="' . basename($fileName) . "\";");
      header('Content-Type: application/octet-stream');
      header('Content-Length: '.$fSize);

      header('Expires: 0');
      header('Cache-Control: must-revalidate');
      header('Pragma: public');
      ob_clean();
      flush();
      readfile($fileName);
      exit;
    }
  } else {
    header($_SERVER['SERVER_PROTOCOL'] . ' 404 Not Found');
    echo 'For filename ' . $fileName;
  }
} else {
  header($_SERVER['SERVER_PROTOCOL'] . ' 403 Forbidden');
}
?>
*/

// http://..../download.php?url=
std::string http::cross_domain = "";

/**
 * Effectue la requete
 */
void http::runRequest(const char* page, int assync) {
  _page = page;
  _status = ST_PENDING;
  _assync = (AssyncMode)assync;
  _progressValue = 0;

  std::string url = cross_domain;
  url += _hostname + _page;

  if (_hostname.size() > 0  && _page.size() > 0) {

    printf("URL : %s\n",url.c_str());
    printf("REQUEST : %s\n", _request == REQUEST_GET ? "GET" : "POST");
    printf("PARAMS : %s\n",_param.c_str());

    if (_targetFileName.size() == 0 ) {
      _targetFileName = format("prepare%d",_uid);
    }

    _handle = emscripten_async_wget2(url.c_str(), _targetFileName.c_str(), _request == REQUEST_GET ? "GET" : "POST", _param.c_str(), this, http::onLoaded, http::onError, http::onProgress);

  } else {
    _error = format("malformed url : %s\n",url.c_str());
    _content = "";
    _status = ST_FAILED;
    _progressValue = -1;
  }
}

/**
* Post
*/
void http::addValue(const char* key, const char* value) {
  if (_param.size() > 0) {
    _param += "&";
    _param += key;
      _param += "=";
    _param += value;
  } else {
    _param += key;
      _param += "=";
    _param += value;
  }
}

void http::onLoaded(const char* file) {
  if (strstr(file,"prepare")) {
    FILE* f = fopen(file,"rb");
    if (f) {
      fseek (f, 0, SEEK_END);
        int size=ftell (f);
        fseek (f, 0, SEEK_SET);

      char* data = new char[size];
      fread(data,size,1,f);
      _content = data;
      delete[] data;
      fclose(f);
    } else {
      _content = file;
    }
  } else {
    _content = file;
  }

  _progressValue = 100;
  _status = ST_OK;
}

void http::onError(int error) {
  printf("onError status: %d\n",error);

  _error = "";
  _content = "";
  _status = ST_FAILED;
  _progressValue = -1;
}

/// TEST
int num_request = 0;
float time_elapsed = 0.0f;

void wait_https() {
  if (num_request == 0) {
    printf("End of all download ... %fs\n",(emscripten_get_now() - time_elapsed) / 1000.f);
    emscripten_cancel_main_loop();
    exit(0);
  }
}

void wait_http(void* request) {
  http* req = reinterpret_cast<http*>(request);
  if (req != 0) {
    if (req->getStatus() == http::ST_PENDING) {
      if (req->getProgress()>0) {
        printf("Progress Request %d : %d\n", req->getId(), req->getProgress());
      }
      emscripten_async_call(wait_http, request, 500);

      } else {
      if (req->getStatus() == http::ST_OK) {
        printf("Success Request %d : %s\n", req->getId(),req->getContent());
      } else if (req->getStatus() == http::ST_ABORTED) {
        printf("Aborted Request %d\n", req->getId());
      } else {
        printf("Error Request %d : %s\n", req->getId(), req->getError());
      }

      num_request --;
    }
  } else {
    num_request --;
  }
}


int main() {
  time_elapsed = emscripten_get_now();

  http* http1 = new http("https://github.com", http::REQUEST_GET, "emscripten_main.zip");
  http1->runRequest("/emscripten-core/emscripten/archive/main.zip", http::ASSYNC_THREAD);

  http* http2 = new http("https://github.com",http::REQUEST_GET, "wolfviking_master.zip");
  http2->runRequest("/wolfviking0/image.js/archive/master.zip", http::ASSYNC_THREAD);
  http2->abortRequest();

  http* http3 = new http("https://raw.github.com", http::REQUEST_GET);
  http3->runRequest("/emscripten-core/emscripten/main/LICENSE", http::ASSYNC_THREAD);

  num_request++;
  emscripten_async_call(wait_http, http1, 500);
  num_request++;
  emscripten_async_call(wait_http, http2, 500);
  num_request++;
  emscripten_async_call(wait_http, http3, 500);

  /*
  Http* http4 = new Http("http://www.---.com",Http::REQUEST_POST);
  http4->addValue("app","123");
  http4->runRequest("/test.php",Http::ASSYNC_THREAD);
  num_request ++;
  emscripten_async_call(wait_http,http4,500);
  */

  emscripten_set_main_loop(wait_https, 0, 0);
  return 0;
}
