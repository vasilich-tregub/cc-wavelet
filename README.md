# Digital wavelet transform (Le Gall 5/3) implemented in C, with Web-like GUI.

The program 'webui-cc-wavelet' uses an http server to attach GUI to the code implementing DWT in C. 
You launch 'webui-cc-wavelet.exe' (Windows) or 'webui-cc-wavelet' (Linux) and open the web page 
index.html in your browser, navigating to the URL `https://localhost:8080`.  
You click an image in the page, and the web page, using xmlhttprequest, sends the image data to the 
Forward DWT function implemented in C which is included with the http server code. The page receives 
the results with the response of this xmlhttprequest and displays the response image data. The button 
'Inverse Transform' calls the inverse DWT function and displays the result thereof.  
The page layout is identical to that of the [WebAssembly wavelet transform](https://vasilich-tregub.github.io/wa-wavelet/index.html).

I started to author this C implementation of the wavelet (Le Gall 5/3) transform with the only purpose 
to compare the performance of C and [Web Assempler](https://vasilich-tregub.github.io/wa-wavelet/index.html)
implementations using the identical algorithm, and soon realized how GUI can be useful to input image data and 
display the results in visuals. 

The http server code of my solution does not implement a full-fledged web server: error processing, 
if exists, is at minimum; headers are parsed only opportunistically, etc. The value displayed in the 
'Performance ms' control on the page includes the delays of auxillary javascript functions -- true 
DWT execution time figures can be read from the http server terminal window. With the goal of this 
application becoming more instructional, use of blocking socket operations and synchronous requests 
is beneficial for those students who just begin to learn web technology. 

This said, the repo shows how you can leverage you browser capabilities by attaching a powerful 
GUI to your programs in a truly cross-platform manner. Great volumes of graphics data can be input 
into your program for processing, streamed off and displayed in this way.

Note to developers: to update the web page files of the project to be copied to the CMAKE 
current binary directory after edit, forcefully save CMakeLists.txt.

