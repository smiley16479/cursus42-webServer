#ifndef	STATUS_LINES_HPP
# define STATUS_LINES_HPP

//1XX: INFORMATION
//MESSAGE:	DESCRIPTION:


const char	*ret_string_100[] = {
"Continue",						//100 The server has received the request headers, and the client should proceed to send the request body
"Switching Protocols",				//101 The requester has asked the server to switch protocols
"Checkpoint",						//102 Used in the resumable requests proposal to resume aborted PUT or POST requests
};
//2XX: SUCCESSFUL
//MESSAGE:	DESCRIPTION:

const char	*ret_string_200[] = {
" OK",								//200 The request is OK (this is the standard response for successful HTTP requests)
" Created",							//201 The request has been fulfilled, and a new resource is created 
" Accepted",						//202 The request has been accepted for processing, but the processing has not been completed
" Non-Authoritative Information",	//203 The request has been successfully processed, but is returning information that may be from another source
" No Content",						//203 The request has been successfully processed, but is not returning any content
" Reset Content",					//204 The request has been successfully processed, but is not returning any content, and requires that the requester reset the document view
" Partial Content"					//205 The server is delivering only part of the resource due to a range header sent by the client
};

// 3XX: REDIRECTION
// MESSAGE:	DESCRIPTION:

const char	*ret_string_300[] = {
" Multiple Choices",				//300 A link list. The user can select a link and go to that location. Maximum five addresses  
" Moved Permanently",				//301 The requested page has moved to a new URL 
" Found",							//302 The requested page has moved temporarily to a new URL 
" See Other",						//303 The requested page can be found under a different URL
" Not Modified",					//304 Indicates the requested page has not been modified since last requested
" Switch Proxy",					//305 No longer used
" Temporary Redirect",				//306 The requested page has moved temporarily to a new URL
" Resume Incomplete"				//307 Used in the resumable requests proposal to resume aborted PUT or POST requests
};

//000 4XX: CLIENT ERROR
//000 MESSAGE:	DESCRIPTION:

const char	*ret_string_400[] = {
" Bad Request",						//400 The request cannot be fulfilled due to bad syntax
" Unauthorized",					//401 The request was a legal request, but the server is refusing to respond to it. For use when authentication is possible but has failed or not yet been provided
" Payment Required",				//402 Reserved for future use
" Forbidden",						//403 The request was a legal request, but the server is refusing to respond to it
" Not Found",						//405 The requested page could not be found but may be available again in the future
" Method Not Allowed",				//406 A request was made of a page using a request method not supported by that page. An origin server MUST generate an Allow field in a 405 (Method Not Allowed)
" Not Acceptable",					//407 The server can only generate a response that is not accepted by the client
" Proxy Authentication Required",	//408 The client must first authenticate itself with the proxy
" Request Timeout",					//409 The server timed out waiting for the request
" Conflict",						//410 The request could not be completed because of a conflict in the request
" Gone",							//411 The requested page is no longer available
" Length Required",					//412 The "Content-Length" is not defined. The server will not accept the request without it 
" Precondition Failed",				//413 The precondition given in the request evaluated to false by the server
" Request Entity Too Large",		//414 The server will not accept the request, because the request entity is too large
" Request-URI Too Long",			//415 The server will not accept the request, because the URL is too long. Occurs when you convert a POST request to a GET request with a long query information 
" Unsupported Media Type",			//416 The server will not accept the request, because the media type is not supported 
" Requested Range Not Satisfiable",	//417 The client has asked for a portion of the file, but the server cannot supply that portion
" Expectation Failed",				//418 The server cannot meet the requirements of the Expect request-header field
};

//000 5XX: SERVER ERROR
//000 MESSAGE:	DESCRIPTION:

const char	*ret_string_500[] = {
" Internal Server Error",			//500 A generic error message, given when no more specific message is suitable
" Not Implemented",					//501 The server either does not recognize the request method, or it lacks the ability to fulfill the request
" Bad Gateway",						//502 The server was acting as a gateway or proxy and received an invalid response from the upstream server
" Service Unavailable",				//503 The server is currently unavailable (overloaded or down)
" Gateway Timeout",					//504 The server was acting as a gateway or proxy and did not receive a timely response from the upstream server
" HTTP Version Not Supported",		//505 The server does not support the HTTP protocol version used in the request
" Network Authentication Required",	//506 The client needs to authenticate to gain network access
};

const char	*ret_string(size_t ret_code)	{
	const char **strings[] = { ret_string_100, ret_string_200, ret_string_300,
								ret_string_400, ret_string_500 };

	return (strings[(ret_code) / 100 - 1][ret_code %100]);
}

#endif
