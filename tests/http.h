//
//  Http.h
//  Player Javascript
//
//  Created by Anthony Liot on 23/11/12.
//

#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdarg.h>
#include <string>


/*
 */
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
		
		// enregistrement sur unigine
    	static void RegisterAsExtension(bool regis);
    	
    	// Callback
		static void onLoaded(unsigned handle, void* parent, const char * file);
		static void onError(unsigned handle, void* parent, int statuserror);
		static void onProgress(unsigned handle, void* parent, int progress);
        
        // Constructeur    
		http(const char* hostname, int requestType, const char* targetFileName = "");
		
		//Destructeur
		virtual ~http();
		
		/**
		* Effectue la requete
		*/
		void runRequest(const char* page, int assync);

		/**
		* Abort the request
		*/
		void abortRequest();

		/**
		* Accede a la reponse
		*/
		const char* getContent();

		/**
		* Accede a l'erreur
		*/
		const char* getError(); 

		/**
		* Accede au status
		*/
		int getStatus();

		/**
		* Accede a la progression
		*/
		float getProgress(); 
		
		/**
		* Get Id of http Class
		*/
		int getId(); 

		/**
		*
		*/
		void addValue(const char* key, const char* value);
		
		/**
		* Callback
		*/
		void onProgress(int progress);
		void onLoaded(const char* file);
		void onError(int error);		
				
		// Static parameter	
		static int uid;
		static std::string cross_domain ;
			
	private:
		
		// Id of request
		int 		_uid;
	
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
		RequestType	_request; 

		// status
		int         _status;

		// progress value
		int         _progressValue;

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
    int nsize = vsnprintf(buffer,size,fmt,vl);
    if(size<=nsize){//fail delete buffer and try again
        delete buffer; buffer = 0;
        buffer = new char[nsize+1];//+1 for /0
        nsize = vsnprintf(buffer,size,fmt,vl);
    }
    std::string ret(buffer);
    va_end(vl);
    delete buffer;
    return ret;
}

#endif /* __HTTP_H__ */
