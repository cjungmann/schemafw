# Why Not Use xsl:include?

The XSL processor in certain browsers cannot handle either _xsl:import_ or _xsl:include_
when the stylesheets are used interactively.  As a result of this, the Schema Framework
will merge stylesheets combined with _xsl:import_ with Javascript.

Merging imported files is easier because the location of the _xsl:import_ elements
is strictly defined and thus predictable.  I have been able to accomplish all my goals
accessing external files using _xsl:import_, so until it proves necessary to
use _xsl:include_, it is better not to use _xsl:include_.

It's possible that a stylesheet that uses _xsl:include_ may work as expected on a
given browser, but it may fail on others.  Unless you're willing to test all the
browers, I recommend avoiding the instruction for now.  

