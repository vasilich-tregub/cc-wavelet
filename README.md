# Digital wavelet transform (Le Gall 5/3) implemented in C, with Web-like GUI.

I authored this C implementation of the wavelet (Le Gall 5/3) transform with the same algorithm that I used 
for my 
[implementation of the wavelet (Le Gall 5/3) transform in Web Assempler](https://vasilich-tregub.github.io/wa-wavelet/index.html)
with the only purpose of comparing their performances. 

The program 'webui-cc-wavelet.exe' uses an http server to attach GUI to the code implementing DWT in C. 
You launch 'webui-cc-wavelet.exe' and open the web page index.html in your browser, navigating to the 
URL `https://localhost:8080`.  
You click an image in the page, and the web page, using http request, sends the image to the Forward 
DWT function implemented in C with the http server code. The page requests the results with the 
xmlhttprequest to the webui-cc-wavelet http server and displays the response.  
The page layout is identical to that of the [WebAssembly wavelet transform](https://vasilich-tregub.github.io/wa-eavelet/index.html).

Note to developers: to update the web page files of the project to be copied to the CMAKE 
current binary directory after edit, forcefully save CMakeLists.txt.

