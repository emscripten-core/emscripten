//
//  http.cpp
//  Player Javascript
//
//  Created by Anthony Liot on 23/11/12.
//

#include "http.h"
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>

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
		

//----------------------------------------------------------------------------------------
// HTTP CLASS
//----------------------------------------------------------------------------------------

void http::onLoaded(unsigned handle, void* parent, const char * file) {
	http* req = reinterpret_cast<http*>(parent);
	req->onLoaded(file);
}

void http::onError(unsigned handle, void* parent, int statuserror) {
	http* req = reinterpret_cast<http*>(parent);
	req->onError(statuserror);
}

void http::onProgress(unsigned handle, void* parent, int progress) {
	http* req = reinterpret_cast<http*>(parent);
	req->onProgress(progress);
}

/**
* Constructeur
*/
http::http(const char* hostname, int requestType, const char* targetFilename) : _hostname(hostname), _page(""), _targetFileName(targetFilename), _param(""), _content(""), _error(""), _request((RequestType)requestType), _status(ST_PENDING), _assync(ASSYNC_THREAD) {
	_progressValue = -1;
	_uid = uid++;
}


/**
* Destructeur
*/
http::~http() {
}

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
		printf("REQUEST : %s\n",(_request==REQUEST_GET) ? "GET":"POST");
		printf("PARAMS : %s\n",_param.c_str());
		
		if (_targetFileName.size() == 0 ) {
			_targetFileName = format("prepare%d",_uid);
		}

		_handle = emscripten_async_wget2(url.c_str(), _targetFileName.c_str(), (_request==REQUEST_GET) ? "GET":"POST", _param.c_str(), this, http::onLoaded, http::onError, http::onProgress);
	
	} else {
		_error = format("malformed url : %s\n",url.c_str());
		_content = "";
		_status = ST_FAILED;
		_progressValue = -1;
	}
}

/**
* Abort the request
*/
void http::abortRequest() {
	emscripten_async_wget2_abort(_handle);
	_status = ST_ABORTED;
}

/**
* Accede a la reponse
*/
const char* http::getContent() {
	return _content.c_str();
}

/**
* Accede a l'erreur
*/
const char* http::getError() {
	return _error.c_str();
}

/**
* Accede au status
*/
int http::getStatus() {
	return _status;
}

/**
* Accede a la progression between 0 & 100
*/
float http::getProgress() {
	return (float)_progressValue;
}

/**
* Accede a la progression between 0 & 100
*/
int http::getId() {
	return _uid;
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

void http::onProgress(int progress) {
	_progressValue = progress;
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
			delete data;
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
	
	printf("Error status : %d\n",error);
	
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
    int result = 0;
    REPORT_RESULT();
	}
}

void wait_http(void* request) {
	http* req = reinterpret_cast<http*>(request); 
	if (req != 0) {
    	if (req->getStatus() == http::ST_PENDING) {
			if ((int)req->getProgress()>0) {
				printf("Progress Request n°%d : %d\n",req->getId(),(int)req->getProgress());
			}
			emscripten_async_call(wait_http,request,500);
			
    	} else {
			if (req->getStatus() == http::ST_OK) { 
				printf("Success Request n°%d : %s\n",req->getId(),req->getContent());
			} else if (req->getStatus() == http::ST_ABORTED) {
				printf("Aborted Request n°%d", req->getId());
			} else {
				printf("Error Request n°%d : %s\n",req->getId(), req->getError());
    		}
    		
  			num_request --;
    	}
  	} else {
		num_request --;  	
  	}
}


int main() {
	time_elapsed = emscripten_get_now();

	http* http1 = new http("https://github.com",http::REQUEST_GET,"emscripten_master.zip");
	http1->runRequest("/kripken/emscripten/archive/master.zip",http::ASSYNC_THREAD);

	http* http2 = new http("https://github.com",http::REQUEST_GET,"wolfviking_master.zip");
	http2->runRequest("/wolfviking0/image.js/archive/master.zip",http::ASSYNC_THREAD);
	http2->abortRequest();

	http* http3 = new http("https://raw.github.com",http::REQUEST_GET);
	http3->runRequest("/kripken/emscripten/master/LICENSE",http::ASSYNC_THREAD);

	num_request ++;
	emscripten_async_call(wait_http,http1,500);
	num_request ++;
	emscripten_async_call(wait_http,http2,500);
	num_request ++;
	emscripten_async_call(wait_http,http3,500);

	/*
	Http* http4 = new Http("http://www.---.com",Http::REQUEST_POST);
	http4->addValue("app","123");
	http4->runRequest("/test.php",Http::ASSYNC_THREAD);
	num_request ++;
	emscripten_async_call(wait_http,http4,500);
	*/
	
    emscripten_set_main_loop(wait_https, 0, 0);
}
