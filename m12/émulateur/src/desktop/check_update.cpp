#include "cJSON/cJSON.h"
#include <string>
#include <regex>
#include <ixwebsocket/IXHttpClient.h>

void parseTagName(std::string& str,int* major,int* minor,int* patch,int* tweak){
	std::regex pattern("\\d+");
	std::smatch matches;
	std::string tmp=str;
	char n=0;
	try{
		while(std::regex_search(tmp, matches, pattern)) {
			switch (n){
				case 0:
					*major=std::stoi(matches[0]);
					break;
				case 1:
					*minor=std::stoi(matches[0]);
					break;
				case 2:
					*patch=std::stoi(matches[0]);
					break;
				case 3:
					*tweak=std::stoi(matches[0]);
					break;
			}
			n++;
			if (n>=4) break;
			tmp=matches.suffix();
		}
	}
	catch(...){
		*major=0;
		*minor=0;
		*patch=0;
		*tweak=0;
	}
}

#if defined(M12_USE_GITHUB_REPO) && defined(M12_USE_TLS)
void getLatestVersionNumber(bool prerelease,int* major,int* minor,int* patch,int* tweak){
	ix::HttpClient httpClient;
	ix::HttpRequestArgsPtr args = httpClient.createRequest();
	args->extraHeaders["Accept"]="application/vnd.github+json";
	ix::HttpResponsePtr response=httpClient.get(
		prerelease?"https://api.github.com/repos/EcrouVis/Minitel/releases?per_page=1&page=1":
				   "https://api.github.com/repos/EcrouVis/Minitel/releases/latest",
		args
	);
	if (response->statusCode==200){
		cJSON* json=cJSON_Parse(response->body.c_str());
		if (json!=NULL){
			if (cJSON_IsArray(json)&&cJSON_GetArraySize(json)!=0){
				cJSON* jtn;
				if (prerelease){
					jtn=cJSON_GetArrayItem(json,0);
					jtn=cJSON_GetObjectItemCaseSensitive(jtn,"tag_name");
				}
				else jtn=cJSON_GetObjectItemCaseSensitive(json,"tag_name");
				if (cJSON_IsString(jtn)){
					std::string tn=jtn->valuestring;
					parseTagName(tn,major,minor,patch,tweak);
				}
			}
			cJSON_Delete(json);
		}
	}
}
#else
void getLatestVersionNumber(bool prerelease,int* major,int* minor,int* patch,int* tweak){
}
#endif