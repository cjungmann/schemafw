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

  <xsl:variable name="merge_num" select="/*/*[@merged][1]/@merged" />

  <!--
  Save a global $mode-type value from merged types or root mode-type value.
  -->
  <xsl:variable name="mtm_result"
                select="/*/*[@rndx][@merged=$merge_num]/@merge-type" />
  <xsl:variable name="mtm_rschema"
                select="/*/*[not($mtm_result)][@rndx][@merged=$merge_num]/schema/@merge-type" />
  <xsl:variable name="mtm_schema"
                select="/*[not($mtm_result|$mtm_rschema)]/schema/@merge-type" />
  <xsl:variable name="mt_root"
                select="/*[not($mtm_result|$mtm_rschema|$mtm_schema)]/@mode-type" />
  <xsl:variable name="mode-type" select="$mtm_result|$mtm_rschema|$mtm_schema|$mt_root" />

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

  <!-- Merged results trump resultset mode-type settings -->
  <xsl:variable name="mer_schema" select="/*/schema[@merged=$merge_num]" />
  <xsl:variable name="mrs_schema" select="/*/*[@rndx][@merged=$merge_num]/schema" />
  <xsl:variable name="mrschema" select="$mer_schema|$mrs_schema" />

  <xsl:variable name="is_merge_form"
                select="$mrschema[starts-with(@merge-type,'form-')]" />
  <xsl:variable name="is_form"
                select="not($is_merge_form) and starts-with($mode-type,'form-')" />
  <!-- <xsl:variable name="is_form" select="starts-with($mode-type,'form-')" /> -->
  
  <xsl:variable
      name="dschema" select="/*[$is_form][not($mrschema)]/schema" />
  <xsl:variable
      name="rschema"
      select="/*[$is_form][not($mrschema|$dschema)]/*[@rndx][schema][1]/schema" />
  <xsl:variable name="gschema" select="($mrschema|$dschema|$rschema)" />

  <!-- $gview, if available, will dictate which result to use. -->
  <xsl:variable name="gsview" select="/*/views/view[@selected]" />
  <xsl:variable name="gfview" select="/*[not($gsview)]/views/view[1]" />
  <xsl:variable name="gview" select="$gsview | $gfview" />

  <!--
  Build $gresult from a view or as a fallback, the first result in the resultset.
  -->
  <xsl:variable
      name="m_res"
      select="/*/*[schema][@rndx][@merge-type]" />
  <xsl:variable
      name="b_res"
      select="/*[not($m_res)]/*[schema][local-name()=/*/@base_result]" />
  <xsl:variable
      name="v_res"
      select="/*[not($m_res|$b_res)][$gview]/*[schema][@rndx][local-name()=$gview/@result]"/>
  <xsl:variable
      name="f_res"
      select="/*[not($m_res|$b_res|$v_res)]/*[schema][@rndx][1]" />

  <xsl:variable name="gresult" select="$m_res|$b_res|$v_res|$f_res" />

</xsl:stylesheet>
