# SchemaFW

**schema** an underlying organizational pattern or structure; conceptual framework
[dictionary.com](http://dictionary.reference.com/browse/schema)

The Schema Framework includes several parts that, working together, define a
protocol by which the server workload is substantially reduced and the client
assumes most of the responsibility for rendering and running the application.

A _schema_ is used to describe how data sent by the server should be interpreted
by the client.

See the [User Guide](userguide/UserGuide.md)

# Installation Steps

## Prerequisites

```
$ sudo apt-get install build-essential
$ sudo apt-get install libmysqlclient-dev
```

# Clone the Repository

Get the SchemaFW source code with the following command:

```
$ git clone https://bitbucket.org/chuckj/schemafw
```

# Build The Project

Build and install the library and executable
```
./configure
make
sudo make install
```

Apache Setup, copy .conf file, then enable it:
```
sudo a2enconf schemafw
sudo /etc/init.d/apache2 restart
```

Setup a new website.  Use `schemafw_setup` to prepare a database and project directory
```
schemafw_setup
```

# Consult the UserGuide to Learn About SchemaFW Programming

[SchemaFW User Guide](userguide/BuildingTheFramework.md)

Programming for SchemaFW applications is done with MySQL stored procedures.
The framework exposes the procedures through script files that instruct
the SchemaFW server which procedures to use and how to use them.

