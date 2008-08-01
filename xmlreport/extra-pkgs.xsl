<?xml version="1.0"?>

<!--
This XSLT program extracts all hosts w/ extra packages.
-->

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml">

  <xsl:output method="text"/>

  <xsl:template match="/report">
    <xsl:text>hosts w/ extra packages installed
=================================

</xsl:text>

    <xsl:choose>
      <xsl:when test="count(group/host/packages/pkg[@extra = '1'])">
        <xsl:for-each select="group">
          <xsl:call-template name="group"/>
        </xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>No hosts with extra packages found!
</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="group">
   <xsl:if test="count(host[packages/pkg[@extra = '1']])">
    <xsl:text>
[</xsl:text><xsl:value-of select="@name"/><xsl:text>]
</xsl:text>

    <xsl:for-each select="host">
      <xsl:if test="count(packages/pkg[@extra = '1'])">
	<xsl:call-template name="host"/>
      </xsl:if>
    </xsl:for-each>
   </xsl:if>
  </xsl:template>

  <xsl:template name="host">
    <xsl:value-of select="@hostname"/><xsl:text>:
</xsl:text>
    <xsl:for-each select="packages/pkg[@extra = '1']">
      <xsl:text>   </xsl:text><xsl:value-of select="@name"/><xsl:text> (</xsl:text><xsl:value-of select="@version"/><xsl:text>)
</xsl:text>
    </xsl:for-each>
    <xsl:text>
</xsl:text> 
 </xsl:template>

</xsl:stylesheet>
