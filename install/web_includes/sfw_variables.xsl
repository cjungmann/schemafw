<?xml version="1.0" encoding="utf-8" ?>

<xsl:stylesheet
   version="1.0"
   xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
   xmlns="http://www.w3.org/1999/xhtml"
   xmlns:html="http://www.w3.org/1999/xhtml"
   exclude-result-prefixes="html">

  <!--
  General explanation of how I conditionally create variables that select nodes
  or node-sets.

  To make variables that hold node or node-set values, I declare variables in
  series with lower priority results negated by the existence of higher-priority
  results.  Creating a variable that conditionally branches with a xsl:choose
  cannot select a node.

  For example, consider $gview.  It is the union of two variables between which
  only one node can be selected.  This is ensured by the lower-priority $gfview
  deferring to $gsview by including the predicate expression [not($gsview)].
  -->

  <!--
  Save a global $mode-type value from merged types or root mode-type value.
  -->
  <xsl:variable name="mtm_result"
                select="/*/*[@rndx]/@merge-type" />
  <xsl:variable name="mtm_schema"
                select="/*[not($mtm_result)]/schema/@merge-type" />
  <xsl:variable name="mt_root"
                select="/*[not($mtm_result|$mtm_schema)]/@mode-type" />
  <xsl:variable name="mode-type" select="$mtm_result|$mtm_schema|$mt_root" />

  <xsl:variable name="result-row"
                select="/*[$mode-type='form-result']/*[@rndx=1]/*[@error]" />

  <!--
  Construct global variables $err_condition, $gview, $gschema, and $gresult
  for use by template fill_host in sfw_host.xsl.
  -->

  <!--
  Create $err_condition variable which acts as a flag to indicate
  that the view is being preempted by an error message.
  -->
  <xsl:variable name="docel_msg" select="/message" />
  <xsl:variable name="child_msg" select="/*[not($docel_msg)]/message" />
  <xsl:variable name="msg-el" select="$docel_msg | $child_msg" />

  <xsl:variable name="err_condition">
    <xsl:choose>
      <xsl:when test="$result-row and $result-row/@error&gt;0">1</xsl:when>
      <xsl:when test="$msg-el and $msg-el/@type='error'">2</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="is_form" select="starts-with($mode-type,'form-')" />
  <xsl:variable
      name="mrschema" select="/*[$is_form]/*[@rndx][@merge-type]/schema" />
  <xsl:variable
      name="msschema" select="/*[$is_form][not($mrschema)]/schema[@merge-type]" />
  <xsl:variable
      name="dschema" select="/*[$is_form][not($mrschema|$msschema)]/schema" />
  <xsl:variable
      name="rschema"
      select="/*[$is_form][not($mrschema|$msschema|$dschema)]/*[@rndx=1]/schema" />
  <xsl:variable name="gschema" select="$mrschema|$msschema|$dschema|$rschema" />

  <!-- $gview, if available, will dictate which result to use. -->
  <xsl:variable name="gsview" select="/*/views/view[@selected]" />
  <xsl:variable name="gfview" select="/*[not($gsview)]/views/view[1]" />
  <xsl:variable name="gview" select="$gsview | $gfview" />

  <!--
  Build $gresult from a view or as a fallback, the first result in the resultset.
  -->
  <xsl:variable
      name="merged_result"
      select="/*/*[@rndx][@merge-type]" />
  <xsl:variable
      name="view_result"
      select="/*[not($merged_result)][$gview]/*[@rndx][local-name()=$gview/@result]"/>
  <xsl:variable
      name="first_result"
      select="/*[not($merged_result|$view_result)]/*[@rndx=1]" />

  <xsl:variable name="gresult" select="$merged_result|$view_result|$first_result" />

</xsl:stylesheet>
