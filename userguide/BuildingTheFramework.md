# Building the Framework

## Install Necessary Applications and Libraries

This guide assumes a clean Debian-based computer, and that the proper repositories
are installed.  Other Linux distributions use other package managers, and are on
their own modifying these instructions to their computers.  Sorry, I'm not 
experienced enough with the other environments to offer much help beyond saying
what's necessary.

Less "clean" machines may not have to install everything below.  You will know your
own machines best and can leave out any or all of the following steps as you see fit.

1. Install _git_
   
   ~~~
   sudo apt-get install git-all
   ~~~
   
   Git is necessary, for now, for getting the source code.  Eventually, I'll
   create a compressed file for download, or perhaps even a deb package, if I
   get fancy.

2. Install Compilers and MySQL Development Library
   
   ~~~
   sudo apt-get install build-essential
   sudo apt-get install libmysqlclient-dev
   ~~~

   These packages are necessary to compile the source code.
    
3. Install mod_fastcgi and libfcgi
   
   ~~~
   sudo apt-get install libfcgi-dev libapache2-mod-fastcgi
   sudo ldconfig
   ~~~
   
   The _mod_fastcgi_ modifies Apache to use FastCGI, and _libfcgi_ is a library
   for creating a FastCGI application.  `sudo ldconfig` may not be necessary, but
   I needed it once, so I've left it in.

   You can also build libfcgi from [source](https://github.com/FastCGI-Archives/FastCGI.com/blob/master/original_snapshot/fcgi-2.4.1-SNAP-0910052249.tar.gz?raw=true).
   In that case, you will definitely need `ldconfig`.
    
## Download, Build, and Install SchemaFW

This is a work in progress.  At the time of this writing (2016-07-13), the C++ code
that generates the server-side document server is mixed up with the
HTML/CSS/Javascript/XSLT files.  These will need to be separated.  The document
server should remain relatively stable and should have its own version number
relative to the still-developing client-side work that will remain more fluid
in the future.

1. Clone the Repository
   
   ~~~
   cd /usr/local/src
   sudo git clone https://chuckj@bitbucket.org/chuckj/schema.git
   ~~~

   I suggest putting the source code in the traditional directory _/usr/local/src_.

2. Configure, Build, and Install

   Assuming we're in _/usr/local/src_ after the completion of step 1:

   ~~~
   cd schema
   sudo ./configure
   sudo make
   sudo make install
   ~~~

3. Confirm That Schema is Running

   ~~~
   schema.fcgi --version
   ~~~

   This should print out the version to confirm the application is running.
   

## Links

Next Page [Configuring Apache](ConfiguringApache.md)

Section [Preparing to Use SchemaFW](PreparingToUseSchemaFW.md)

[Main Page](UserGuide.md)

