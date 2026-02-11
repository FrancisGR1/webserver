#include <string>
#include <map>

#include "utils.hpp"
#include "MimeTypes.hpp"

// https://github.com/nginx/nginx/blob/master/conf/mime.types
const std::map<std::string, std::string>& MimeTypes::mimes()
{
	static const std::map<std::string, std::string>& mimes = make_mimes();
	return mimes;
}

std::map<std::string, std::string> MimeTypes::make_mimes()
{
	std::map<std::string, std::string> m;

	m["html"]  = "text/html";
	m["htm "]  = "text/html";
	m["shtml"] = "text/html";
	m["css"]   = "text/css";
	m["xml"]   = "text/xml";
	m["gif"]   = "image/gif";
	m["jpeg"]  = "image/jpeg";
	m["jpg"]   = "image/jpeg";
	m["js"]    = "application/javascript";
	m["atom"]  = "application/atom+xml";
	m["rss"]   = "application/rss+xml";

	m["mml"] = "text/mathml";
	m["txt"] = "text/plain";
	m["jad"] = "text/vnd.sun.j2me.app-descriptor";
	m["wml"] = "text/vnd.wap.wml";
	m["htc"] = "text/x-component";

	m["avif"] = "image/avif";
	m["png"]  = "image/png";
	m["svg"]  = "image/svg+xml";
	m["svgz"] = "image/svg+xml";
	m["tif"]  = "image/tiff";
	m["tiff"] = "image/tiff";
	m["wbmp"] = "image/vnd.wap.wbmp";
	m["webp"] = "image/webp";
	m["ico"]  = "image/x-icon";
	m["jng"]  = "image/x-jng";
	m["bmp"]  = "image/x-ms-bmp";

	m["woff"]  = "font/woff";
	m["woff2"] = "font/woff2";

	m["jar"]     = "application/java-archive";
	m["war"]     = "application/java-archive";
	m["ear"]     = "application/java-archive";
	m["json"]    = "application/json";
	m["hqx"]     = "application/mac-binhex40";
	m["doc"]     = "application/msword";
	m["pdf"]     = "application/pdf";
	m["ai"]      = "application/postscript";
	m["ps"]      = "application/postscript";
	m["ai"]      = "application/postscript";
	m["rtf"]     = "application/rtf";
	m["m3u8"]    = "application/vnd.apple.mpegurl";
	m["kml"]     = "application/vnd.google-earth.kml+xml";
	m["kmz"]     = "application/vnd.google-earth.kmz";
	m["xls"]     = "application/vnd.ms-excel";
	m["eot"]     = "application/vnd.ms-fontobject";
	m["ppt"]     = "application/vnd.ms-powerpoint";
	m["odg"]     = "application/vnd.oasis.opendocument.graphics";
	m["odp"]     = "application/vnd.oasis.opendocument.presentation";
	m["ods"]     = "application/vnd.oasis.opendocument.spreadsheet";
	m["odt"]     = "application/vnd.oasis.opendocument.text";
	m["pptx"]    = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	m["xlsx"]    = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	m["docx"]    = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	m["wmlc"]    = "application/vnd.wap.wmlc";
	m["wasm"]    = "application/wasm";
	m["7z"]      = "application/x-7z-compressed";
	m["cco"]     = "application/x-cocoa";
	m["jardiff"] = "application/x-java-archive-diff";
	m["jnlp"]    = "application/x-java-jnlp-file";
	m["run"]     = "application/x-makeself";
	m["pl"]      = "application/x-perl";
	m["pm"]      = "application/x-perl";
	m["prc"]     = "application/x-pilot";
	m["pdb"]     = "application/x-pilot";
	m["rar"]     = "application/x-rar-compressed";
	m["rpm"]     = "application/x-redhat-package-manager";
	m["sea"]     = "application/x-sea";
	m["swf"]     = "application/x-shockwave-flash";
	m["sit"]     = "application/x-stuffit";
	m["tk"]      = "application/x-tcl";
	m["pem"]     = "application/x-x509-ca-cert";
	m["der"]     = "application/x-x509-ca-cert";
	m["crt"]     = "application/x-x509-ca-cert";
	m["xpi"]     = "application/x-xpinstall";
	m["xhtml"]   = "application/xhtml+xml";
	m["xspf"]    = "application/xspf+xml";
	m["zip"]     = "application/zip";
	
	m["exe"]  = "application/octet-stream";
	m["bin"]  = "application/octet-stream";
	m["dll"]  = "application/octet-stream";
	m["deb"]  = "application/octet-stream";
	m["dmg"]  = "application/octet-stream";
	m["img"]  = "application/octet-stream";
	m["iso"]  = "application/octet-stream";
	m["msp"]  = "application/octet-stream";
	m["msi"]  = "application/octet-stream";
	m["msm"]  = "application/octet-stream";
	m["mid"]  = "audio/midi";
	m["midi"] = "audio/midi";
	m["kar"]  = "audio/midi";
	m["mp3"]  = "audio/mpeg";
	m["ogg"]  = "audio/ogg";
	m["m4a"]  = "audio/x-m4a";
	m["ra"]   = "audio/x-realaudio";

	m["3gp"]  = "video/3gpp";
	m["3gpp"] = "video/3gpp";
	m["ts"]   = "video/mp2t";
	m["mp4"]  = "video/mp4";
	m["mpg"]  = "video/mpeg";
	m["mpeg"] = "video/mpeg";
	m["mov"]  = "video/quicktime";
	m["webm"] = "video/webm";
	m["flv"]  = "video/x-flv";
	m["m4v"]  = "video/x-m4v";
	m["mng"]  = "video/x-mng";
	m["asf"]  = "video/x-ms-asf";
	m["asf"]  = "video/x-ms-asx";
	m["wmv"]  = "video/x-ms-wmv";
	m["avi"]  = "video/x-msvideo";

	return m;
}

std::string MimeTypes::from_path(const std::string& path)
{
	size_t dot = path.find_last_of('.');
	if (dot == std::string::npos)
		return "application/octet-stream";

	std::string ext = utils::str_tolower(path.substr(dot + 1));

	const std::map<std::string, std::string>& m = mimes();
	std::map<std::string, std::string>::const_iterator it = m.find(ext);
	if (it != m.end())
		return it->second;

	//@NOTE: default mime type
	return "application/octet-stream";
}
