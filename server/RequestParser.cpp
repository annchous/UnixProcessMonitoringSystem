#include <string>
#include <vector>

enum RequestType {
    GET,
    PUT,
    POST,
    DELETE
};

struct RequestBody {
    RequestType requestType{};
    std::vector<std::string> args{};
    std::string body;
};

class RequestParser {
public:
    RequestBody parse(const std::string& request) {
        std::vector<std::string> out = split(request, ' ');

        if(out.size() != 3)
            throw std::exception();
        
        RequestBody requestBody;
        if(out[0] == "GET") 
			requestBody.requestType = GET;
        else if (out[0] == "POST") 
			requestBody.requestType = POST;
        else 
			throw std::exception();
			
        auto tmp = split(out[1], '/');
        tmp.erase(tmp.begin());
        requestBody.args = tmp;
        
        return requestBody;
    }

    static std::vector<std::string> split(std::string str, char sym) {
        std::vector<std::string> vect;
        std::string tmp;
        for (char i : str){
            if (i == sym) {
                vect.push_back(tmp);
                tmp = "";
                continue;
            } else {
                tmp += i;
            }
        }

        if (!tmp.empty()) vect.push_back(tmp);

        return vect;
    }
};
