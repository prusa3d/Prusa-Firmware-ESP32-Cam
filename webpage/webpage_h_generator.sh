#!/bin/bash

original_webpage_path=../ESP32_PrusaConnectCam/WebPage.h
new_webpage_path=./WebPage.h

echo "Starting the WebPage.h generator"

echo "Copying the original WebPage.h file to the new location"
cp ${original_webpage_path} ${new_webpage_path}

# Read the index.html file and generate the WebPage.h file
echo "Generating index.html"
html_content=$(cat index.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char index_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char index_html\[\] PROGMEM = R"rawliteral\(/) { print "const char index_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the page_auth.html file and generate the WebPage.h file
echo "Generating page_auth.html"
html_content=$(cat page_auth.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char page_auth_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char page_auth_html\[\] PROGMEM = R"rawliteral\(/) { print "const char page_auth_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the page_wifi.html file and generate the WebPage.h file
echo "Generating page_wifi.html"
html_content=$(cat page_wifi.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char page_wifi_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char page_wifi_html\[\] PROGMEM = R"rawliteral\(/) { print "const char page_wifi_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the page_config.html file and generate the WebPage.h file
echo "Generating page_config.html"
html_content=$(cat page_config.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char page_config_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char page_config_html\[\] PROGMEM = R"rawliteral\(/) { print "const char page_config_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the page_system.html file and generate the WebPage.h file
echo "Generating page_system.html"
html_content=$(cat page_system.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char page_system_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char page_system_html\[\] PROGMEM = R"rawliteral\(/) { print "const char page_system_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the syles.css file and generate the WebPage.h file
echo "Generating styles.css"
html_content=$(cat styles.css | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char styles_css\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char styles_css\[\] PROGMEM = R"rawliteral\(/) { print "const char styles_css[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the scripts.js file and generate the WebPage.h file
echo "Generating scripts.js"
html_content=$(cat scripts.js | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char scripts_js\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char scripts_js\[\] PROGMEM = R"rawliteral\(/) { print "const char scripts_js[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the license.html file and generate the WebPage.h file
echo "Generating license.html"
html_content=$(cat license.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char license_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char license_html\[\] PROGMEM = R"rawliteral\(/) { print "const char license_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the gtac.html file and generate the WebPage.h file
echo "Generating gtac.html"
html_content=$(cat gtac.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char gtac_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char gtac_html\[\] PROGMEM = R"rawliteral\(/) { print "const char gtac_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the privacypolicy.html file and generate the WebPage.h file
echo "Generating privacypolicy.html"
html_content=$(cat privacypolicy.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char privacypolicy_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char privacypolicy_html\[\] PROGMEM = R"rawliteral\(/) { print "const char privacypolicy_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Read the cookies.html file and generate the WebPage.h file
echo "Generating cookies.html"
html_content=$(cat cookies.html | awk '{printf "%s\\n", $0}')
awk -v var="$html_content" '/^const char cookies_html\[\] PROGMEM = R"rawliteral\(/,/rawliteral";/ { if (/^const char cookies_html\[\] PROGMEM = R"rawliteral\(/) { print "const char cookies_html[] PROGMEM = R\"rawliteral(\n" var ")rawliteral\";"; next } { next } } 1' WebPage.h > temp && mv temp WebPage.h

# Copy the generated WebPage.h file to the ESP32 project
echo "WebPage.h generated successfully"
echo "Copy the new WebPage.h file to the ESP32 project"
cp  ${new_webpage_path} ${original_webpage_path}
rm -f ${new_webpage_path}

exit 0