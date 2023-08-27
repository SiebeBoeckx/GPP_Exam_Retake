start GPP_TEST_RELEASE.exe -x 30 -y 30 -s 4

start GPP_TEST_RELEASE.exe -x 1000 -y 30 -s 5

start GPP_TEST_RELEASE.exe -x 30 -y 540 -s 30

start GPP_TEST_RELEASE.exe -x 1000 -y 540 -s 3
echo new ActiveXObject("WScript.Shell").AppActivate("GPP_TEST_RELEASE.exe"); > tmp.js
cscript //nologo tmp.js & del tmp.js