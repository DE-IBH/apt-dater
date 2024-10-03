<?xml version="1.0"?>

<!--
This XSLT program prints an overview of the status of all hosts.
-->

<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns="http://www.w3.org/1999/xhtml">

  <xsl:output method="text"/>

  <xsl:template match="/report">
    <xsl:text>Hostname;status;Distribution;Version;Codename;Kernel;Reboot_needed;Number_of_packages_to_update;Packages_with_Version_changes_for_update;Number_of_packages_not_from_used_repos;
</xsl:text>

    <xsl:for-each select="group">
      <xsl:call-template name="group"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="group">
  <!--
    <xsl:text>
[</xsl:text><xsl:value-of select="@name"/><xsl:text>]
    </xsl:text>
  -->

    <xsl:for-each select="host">
	<xsl:call-template name="host"/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="host">
    <xsl:value-of select="@hostname"/><xsl:text>;</xsl:text>
    <xsl:value-of select="status"/><xsl:text>;</xsl:text>
    <xsl:if test="status != 'Unknown'">
      <xsl:value-of select="lsb/distri"/><xsl:text>;</xsl:text>
      <xsl:value-of select="lsb/release"/><xsl:text>;</xsl:text>
      <xsl:value-of select="lsb/codename"/><xsl:text>;</xsl:text>
    <xsl:value-of select="kernel"/><xsl:text>;</xsl:text>
    <xsl:if test="kernel/@reboot = '1'">
      <xsl:text>1;</xsl:text>
    </xsl:if>
    <xsl:if test="not(kernel/@reboot = '1')">
      <xsl:text>0;</xsl:text>
    </xsl:if>
    <xsl:value-of select="count(packages/pkg[@hasupdate = '1'])"/>
    <xsl:text>;</xsl:text>
    <xsl:for-each select="packages/pkg">
      <xsl:if test="@hasupdate = '1'">
        <xsl:value-of select="@name"/>
        <xsl:text> (</xsl:text>
        <xsl:value-of select="@version"/>
        <xsl:text> </xsl:text>
        <xsl:value-of select="@data"/>
        <xsl:text>), </xsl:text>
    </xsl:if>
  </xsl:for-each>
  <xsl:text>;</xsl:text>
  <xsl:value-of select="count(packages/pkg[@extra = '1'])"/>
  <xsl:text>;</xsl:text>
    </xsl:if>

    <xsl:text>
</xsl:text>
  </xsl:template>

</xsl:stylesheet>
