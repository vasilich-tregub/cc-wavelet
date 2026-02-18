# Digital wavelet transform (Le Gall 5/3) implemented in C, with Web-like GUI.

The program 'webui-cc-wavelet.exe' uses an http server to attach GUI to the code implementing DWT in C. 
You launch 'webui-cc-wavelet.exe' and open the web page index.html in your browser, navigating to the 
URL `https://localhost:8080`.  
You click an image in the page, and the web page via http request sends the image to the Forward 
DWT function implemented in C with the http server code. The page requests the results with the 
xmlhttprequest to the webui-cc-wavelet http server and displays the response. The button 'Inverse 
Transform' calls the inverse DWT function and displays the result thereof.  
The page layout is identical to that of the [WebAssembly wavelet transform](https://vasilich-tregub.github.io/wa-wavelet/index.html).

I authored this C implementation of the wavelet (Le Gall 5/3) transform with the same algorithm that I used 
for my 
[implementation of the wavelet (Le Gall 5/3) transform in Web Assempler](https://vasilich-tregub.github.io/wa-wavelet/index.html)
with the only purpose of comparing the performances of these two implementations. 

The http server code does not implement a full-fledged web server: error processing, if exists, is 
at minimum; headers are parsed only a propos, etc. The value displayed in the 'Performance ms' control 
on the page includes the delays of javascript auxillary functions; pure DWT execution time figures 
cna be read from the http server terminal window. Also, this repo shows how you can leverage you 
browser capabilities to attach a powerful GUI to your programs in a truly cross-platform manner. 
Great volumes of graphics data can be input into your program, streamed off and displayed in this way.

Note to developers: to update the web page files of the project to be copied to the CMAKE 
current binary directory after edit, forcefully save CMakeLists.txt.

