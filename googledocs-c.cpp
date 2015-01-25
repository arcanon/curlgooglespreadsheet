#include <curl/curl.h>

using namespace std;
char g_auth[500];
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	cout << " data: "<< ptr << endl << flush;
	char * currentAuth = strstr(ptr,"Auth=");
	if (currentAuth)
	{
		memcpy(g_auth,currentAuth+5,strlen(currentAuth+5));
		g_auth[strlen(currentAuth+5)-1] = 0;
		cout << " authfound: " << g_auth << endl << flush;
	}
	return size*nmemb;
}

void writeDataGoogle (Sensor * s)
{
	printf("writing data google\n");
	CURL *curl;
	CURLcode res;
 
	g_auth[0] = 0;
	
	std::string buffer;
	curl = curl_easy_init();
	if(curl) {
		cout << " curl_easy_init succeeded "<< endl << flush;
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/accounts/ClientLogin");
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "Email=YOUREMAIL&Passwd=YOURPASSWORD&accountType=GOOGLE&source=cURL&service=writely");
		//res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
		
		res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		if (res != CURLE_OK)
		{
			fprintf(stderr, "failed to set write data [%s]\n", "");
 
			return;
		}
		cout << " curl_easy_perform " << endl << flush;
		res = curl_easy_perform(curl);
		cout << " curl_easy_perform done "<< endl << flush;
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));
		}
		else
			printf("curl_easy_perform success \n");
		
		if (strlen(g_auth) > 0)
		{
			cout << " uploading data" << endl << flush;
			
			char postData[800];
			time_t ltime;
			struct tm * curtime;
			char dateStr[80];
			char timeStr[80];

			time( &ltime );
			curtime = localtime( &ltime );
			strftime(dateStr,80,"%F", curtime);
			strftime(timeStr,80,"%T", curtime);
			
			sprintf(postData, "<entry xmlns='http://www.w3.org/2005/Atom' xmlns:gsx='http://schemas.google.com/spreadsheets/2006/extended'> <gsx:date>%s</gsx:date>  <gsx:time>%s</gsx:time>  <gsx:temp>%f</gsx:temp>  <gsx:relhum>%f</gsx:relhum><gsx:location>%d</gsx:location><gsx:device>oregon</gsx:device></entry>", 
			dateStr, timeStr, s->getTemperature(), s->getHumidity(), s->getChannel());
			
			cout << "xml " << postData << endl << flush;
			
			curl_easy_setopt(curl, CURLOPT_URL, "https://spreadsheets.google.com/feeds/list/YOURSPREADSHEETKEY/1/private/full");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(postData));
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
			curl_easy_setopt(curl, CURLOPT_POST, true);
			
			struct curl_slist *headerlist=NULL;
			char authHeader[600];
			sprintf(authHeader, "Authorization: GoogleLogin auth=%s", g_auth);
			cout << "auth header " << authHeader << endl << flush;
			headerlist = curl_slist_append(headerlist, authHeader);
			headerlist = curl_slist_append(headerlist, "GData-Version: 3.0");
			headerlist = curl_slist_append(headerlist, "Content-Type: application/atom+xml");
			
			curl_easy_setopt(curl,CURLOPT_HTTPHEADER , headerlist );
			res = curl_easy_perform(curl);
			cout << " curl_easy_perform done "<< endl << flush;
			/* Check for errors */ 
			if(res != CURLE_OK)
			{				
			  fprintf(stderr, "curl_easy_perform() failed: %s\n",
					  curl_easy_strerror(res));
			}
			else
				printf("curl_easy_perform success \n");
		}
	 
		/* always cleanup */ 
		curl_easy_cleanup(curl);
		
		cout << " buffer " << buffer << flush;
	}
	
	cout << " end google " << buffer << flush;
	
}

#include <sys/time.h>

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}