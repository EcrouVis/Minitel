#ifndef LICENSE_H
#define LICENSE_H

struct License{
	const char* title;
	const char* content;
};

constexpr char lib_imgui[]="Dear ImGui";
constexpr char license_imgui[]="\
Copyright (c) 2014-2025 Omar Cornut\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all\n\
copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\
SOFTWARE.";

constexpr char lib_glfw[]="GLFW";
constexpr char license_glfw[]="\
Copyright © 2002-2006 Marcus Geelnard\n\
\n\
Copyright © 2006-2019 Camilla Löwy\n\
\n\
This software is provided ‘as-is’, without any express or implied warranty. In no event\n\
will the authors be held liable for any damages arising from the use of this software.\n\
\n\
Permission is granted to anyone to use this software for any purpose, including\n\
commercial applications, and to alter it and redistribute it freely, subject to the\n\
following restrictions:\n\
\n\
	The origin of this software must not be misrepresented; you must not claim that you\n\
	wrote the original software. If you use this software in a product, an acknowledgment\n\
	in the product documentation would be appreciated but is not required.\n\
\n\
	Altered source versions must be plainly marked as such, and must not be misrepresented\n\
	as being the original software.\n\
\n\
	This notice may not be removed or altered from any source distribution.";

constexpr char lib_miniaudio[]="miniaudio";
constexpr char license_miniaudio[]="\
Copyright 2025 David Reid\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy of\n\
this software and associated documentation files (the \"Software\"), to deal in\n\
the Software without restriction, including without limitation the rights to\n\
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies\n\
of the Software, and to permit persons to whom the Software is furnished to do\n\
so.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\
SOFTWARE.";

constexpr char lib_cjson[]="cJSON";
constexpr char license_cjson[]="\
Copyright (c) 2009-2017 Dave Gamble and cJSON contributors\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in\n\
all copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\n\
THE SOFTWARE.";

constexpr char lib_ixwebsocket[]="IXWebSocket";
constexpr char license_ixwebsocket[]="\
Copyright (c) 2018 Machine Zone, Inc. All rights reserved.\n\
\n\
Redistribution and use in source and binary forms, with or without\n\
modification, are permitted provided that the following conditions are\n\
met:\n\
\n\
1. Redistributions of source code must retain the above copyright\n\
   notice, this list of conditions and the following disclaimer.\n\
\n\
2. Redistributions in binary form must reproduce the above copyright\n\
   notice, this list of conditions and the following disclaimer in the\n\
   documentation and/or other materials provided with the\n\
   distribution.\n\
\n\
3. Neither the name of the copyright holder nor the names of its\n\
   contributors may be used to endorse or promote products derived\n\
   from this software without specific prior written permission.\n\
\n\
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n\
\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n\
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n\
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\n\
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n\
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\n\
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n\
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n\
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n\
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n\
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

constexpr char font_proggyclean[]="ProggyClean";
constexpr char license_proggyclean[]="\
MIT License\n\
\n\
Copyright (c) 2004, 2005 Tristan Grimmer\n\
\n\
Permission is hereby granted, free of charge, to any person obtaining a copy\n\
of this software and associated documentation files (the \"Software\"), to deal\n\
in the Software without restriction, including without limitation the rights\n\
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n\
copies of the Software, and to permit persons to whom the Software is\n\
furnished to do so, subject to the following conditions:\n\
\n\
The above copyright notice and this permission notice shall be included in all\n\
copies or substantial portions of the Software.\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n\
SOFTWARE.";

constexpr char font_fontawesome[]="Font Awesome";
constexpr char license_fontawesome[]="\
SIL OPEN FONT LICENSE\n\
Version 1.1 - 26 February 2007\n\
\n\
PREAMBLE\n\
The goals of the Open Font License (OFL) are to stimulate worldwide\n\
development of collaborative font projects, to support the font creation\n\
efforts of academic and linguistic communities, and to provide a free and\n\
open framework in which fonts may be shared and improved in partnership\n\
with others.\n\
\n\
The OFL allows the licensed fonts to be used, studied, modified and\n\
redistributed freely as long as they are not sold by themselves. The\n\
fonts, including any derivative works, can be bundled, embedded,\n\
redistributed and/or sold with any software provided that any reserved\n\
names are not used by derivative works. The fonts and derivatives,\n\
however, cannot be released under any other type of license. The\n\
requirement for fonts to remain under this license does not apply\n\
to any document created using the fonts or their derivatives.\n\
\n\
DEFINITIONS\n\
\"Font Software\" refers to the set of files released by the Copyright\n\
Holder(s) under this license and clearly marked as such. This may\n\
include source files, build scripts and documentation.\n\
\n\
\"Reserved Font Name\" refers to any names specified as such after the\n\
copyright statement(s).\n\
\n\
\"Original Version\" refers to the collection of Font Software components as\n\
distributed by the Copyright Holder(s).\n\
\n\
\"Modified Version\" refers to any derivative made by adding to, deleting,\n\
or substituting — in part or in whole — any of the components of the\n\
Original Version, by changing formats or by porting the Font Software to a\n\
new environment.\n\
\n\
\"Author\" refers to any designer, engineer, programmer, technical\n\
writer or other person who contributed to the Font Software.\n\
\n\
PERMISSION & CONDITIONS\n\
Permission is hereby granted, free of charge, to any person obtaining\n\
a copy of the Font Software, to use, study, copy, merge, embed, modify,\n\
redistribute, and sell modified and unmodified copies of the Font\n\
Software, subject to the following conditions:\n\
\n\
1) Neither the Font Software nor any of its individual components,\n\
in Original or Modified Versions, may be sold by itself.\n\
\n\
2) Original or Modified Versions of the Font Software may be bundled,\n\
redistributed and/or sold with any software, provided that each copy\n\
contains the above copyright notice and this license. These can be\n\
included either as stand-alone text files, human-readable headers or\n\
in the appropriate machine-readable metadata fields within text or\n\
binary files as long as those fields can be easily viewed by the user.\n\
\n\
3) No Modified Version of the Font Software may use the Reserved Font\n\
Name(s) unless explicit written permission is granted by the corresponding\n\
Copyright Holder. This restriction only applies to the primary font name as\n\
presented to the users.\n\
\n\
4) The name(s) of the Copyright Holder(s) or the Author(s) of the Font\n\
Software shall not be used to promote, endorse or advertise any\n\
Modified Version, except to acknowledge the contribution(s) of the\n\
Copyright Holder(s) and the Author(s) or with their explicit written\n\
permission.\n\
\n\
5) The Font Software, modified or unmodified, in part or in whole,\n\
must be distributed entirely under this license, and must not be\n\
distributed under any other license. The requirement for fonts to\n\
remain under this license does not apply to any document created\n\
using the Font Software.\n\
\n\
TERMINATION\n\
This license becomes null and void if any of the above conditions are\n\
not met.\n\
\n\
DISCLAIMER\n\
THE FONT SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n\
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OF\n\
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT\n\
OF COPYRIGHT, PATENT, TRADEMARK, OR OTHER RIGHT. IN NO EVENT SHALL THE\n\
COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,\n\
INCLUDING ANY GENERAL, SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL\n\
DAMAGES, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n\
FROM, OUT OF THE USE OR INABILITY TO USE THE FONT SOFTWARE OR FROM\n\
OTHER DEALINGS IN THE FONT SOFTWARE.";

constexpr char lib_libressl[]="LibreSSL";
constexpr char license_libressl[]="\
LibreSSL files are retained under the copyright of the authors. New\n\
  additions are ISC licensed as per OpenBSD's normal licensing policy,\n\
  or are placed in the public domain.\n\
\n\
  The OpenSSL code is distributed under the terms of the original OpenSSL\n\
  licenses which follow:\n\
\n\
  LICENSE ISSUES\n\
  ==============\n\
\n\
  The OpenSSL toolkit stays under a dual license, i.e. both the conditions of\n\
  the OpenSSL License and the original SSLeay license apply to the toolkit.\n\
  See below for the actual license texts.  In case of any license issues\n\
  related to OpenSSL please contact openssl-core@openssl.org.\n\
\n\
  OpenSSL License\n\
  ---------------\n\
\n\
/* ====================================================================\n\
 * Copyright (c) 1998-2011 The OpenSSL Project.  All rights reserved.\n\
 *\n\
 * Redistribution and use in source and binary forms, with or without\n\
 * modification, are permitted provided that the following conditions\n\
 * are met:\n\
 *\n\
 * 1. Redistributions of source code must retain the above copyright\n\
 *    notice, this list of conditions and the following disclaimer.\n\
 *\n\
 * 2. Redistributions in binary form must reproduce the above copyright\n\
 *    notice, this list of conditions and the following disclaimer in\n\
 *    the documentation and/or other materials provided with the\n\
 *    distribution.\n\
 *\n\
 * 3. All advertising materials mentioning features or use of this\n\
 *    software must display the following acknowledgment:\n\
 *    \"This product includes software developed by the OpenSSL Project\n\
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)\"\n\
 *\n\
 * 4. The names \"OpenSSL Toolkit\" and \"OpenSSL Project\" must not be used to\n\
 *    endorse or promote products derived from this software without\n\
 *    prior written permission. For written permission, please contact\n\
 *    openssl-core@openssl.org.\n\
 *\n\
 * 5. Products derived from this software may not be called \"OpenSSL\"\n\
 *    nor may \"OpenSSL\" appear in their names without prior written\n\
 *    permission of the OpenSSL Project.\n\
 *\n\
 * 6. Redistributions of any form whatsoever must retain the following\n\
 *    acknowledgment:\n\
 *    \"This product includes software developed by the OpenSSL Project\n\
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)\"\n\
 *\n\
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY\n\
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n\
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR\n\
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR\n\
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\n\
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n\
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n\
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n\
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\n\
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)\n\
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED\n\
 * OF THE POSSIBILITY OF SUCH DAMAGE.\n\
 * ====================================================================\n\
 *\n\
 * This product includes cryptographic software written by Eric Young\n\
 * (eay@cryptsoft.com).  This product includes software written by Tim\n\
 * Hudson (tjh@cryptsoft.com).\n\
 *\n\
 */\n\
\n\
 Original SSLeay License\n\
 -----------------------\n\
\n\
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)\n\
 * All rights reserved.\n\
 *\n\
 * This package is an SSL implementation written\n\
 * by Eric Young (eay@cryptsoft.com).\n\
 * The implementation was written so as to conform with Netscapes SSL.\n\
 *\n\
 * This library is free for commercial and non-commercial use as long as\n\
 * the following conditions are aheared to.  The following conditions\n\
 * apply to all code found in this distribution, be it the RC4, RSA,\n\
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation\n\
 * included with this distribution is covered by the same copyright terms\n\
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).\n\
 *\n\
 * Copyright remains Eric Young's, and as such any Copyright notices in\n\
 * the code are not to be removed.\n\
 * If this package is used in a product, Eric Young should be given attribution\n\
 * as the author of the parts of the library used.\n\
 * This can be in the form of a textual message at program startup or\n\
 * in documentation (online or textual) provided with the package.\n\
 *\n\
 * Redistribution and use in source and binary forms, with or without\n\
 * modification, are permitted provided that the following conditions\n\
 * are met:\n\
 * 1. Redistributions of source code must retain the copyright\n\
 *    notice, this list of conditions and the following disclaimer.\n\
 * 2. Redistributions in binary form must reproduce the above copyright\n\
 *    notice, this list of conditions and the following disclaimer in the\n\
 *    documentation and/or other materials provided with the distribution.\n\
 * 3. All advertising materials mentioning features or use of this software\n\
 *    must display the following acknowledgement:\n\
 *    \"This product includes cryptographic software written by\n\
 *     Eric Young (eay@cryptsoft.com)\"\n\
 *    The word 'cryptographic' can be left out if the rouines from the library\n\
 *    being used are not cryptographic related :-).\n\
 * 4. If you include any Windows specific code (or a derivative thereof) from\n\
 *    the apps directory (application code) you must include an acknowledgement:\n\
 *    \"This product includes software written by Tim Hudson (tjh@cryptsoft.com)\"\n\
 *\n\
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND\n\
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n\
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n\
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE\n\
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n\
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n\
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n\
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n\
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n\
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n\
 * SUCH DAMAGE.\n\
 *\n\
 * The licence and distribution terms for any publically available version or\n\
 * derivative of this code cannot be changed.  i.e. this code cannot simply be\n\
 * copied and put under another distribution licence\n\
 * [including the GNU Public Licence.]\n\
 */";

constexpr char lib_openssl[]="OpenSSL";
constexpr char license_openssl[]="\
Copyright (c) 1998-2026 The OpenSSL Project Authors\n\
\n\
Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson\n\
\n\
All rights reserved.\n\
                                 Apache License\n\
                           Version 2.0, January 2004\n\
                        https://www.apache.org/licenses/\n\
\n\
   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION\n\
\n\
   1. Definitions.\n\
\n\
      \"License\" shall mean the terms and conditions for use, reproduction,\n\
      and distribution as defined by Sections 1 through 9 of this document.\n\
\n\
      \"Licensor\" shall mean the copyright owner or entity authorized by\n\
      the copyright owner that is granting the License.\n\
\n\
      \"Legal Entity\" shall mean the union of the acting entity and all\n\
      other entities that control, are controlled by, or are under common\n\
      control with that entity. For the purposes of this definition,\n\
      \"control\" means (i) the power, direct or indirect, to cause the\n\
      direction or management of such entity, whether by contract or\n\
      otherwise, or (ii) ownership of fifty percent (50%) or more of the\n\
      outstanding shares, or (iii) beneficial ownership of such entity.\n\
\n\
      \"You\" (or \"Your\") shall mean an individual or Legal Entity\n\
      exercising permissions granted by this License.\n\
\n\
      \"Source\" form shall mean the preferred form for making modifications,\n\
      including but not limited to software source code, documentation\n\
      source, and configuration files.\n\
\n\
      \"Object\" form shall mean any form resulting from mechanical\n\
      transformation or translation of a Source form, including but\n\
      not limited to compiled object code, generated documentation,\n\
      and conversions to other media types.\n\
\n\
      \"Work\" shall mean the work of authorship, whether in Source or\n\
      Object form, made available under the License, as indicated by a\n\
      copyright notice that is included in or attached to the work\n\
      (an example is provided in the Appendix below).\n\
\n\
      \"Derivative Works\" shall mean any work, whether in Source or Object\n\
      form, that is based on (or derived from) the Work and for which the\n\
      editorial revisions, annotations, elaborations, or other modifications\n\
      represent, as a whole, an original work of authorship. For the purposes\n\
      of this License, Derivative Works shall not include works that remain\n\
      separable from, or merely link (or bind by name) to the interfaces of,\n\
      the Work and Derivative Works thereof.\n\
\n\
      \"Contribution\" shall mean any work of authorship, including\n\
      the original version of the Work and any modifications or additions\n\
      to that Work or Derivative Works thereof, that is intentionally\n\
      submitted to Licensor for inclusion in the Work by the copyright owner\n\
      or by an individual or Legal Entity authorized to submit on behalf of\n\
      the copyright owner. For the purposes of this definition, \"submitted\"\n\
      means any form of electronic, verbal, or written communication sent\n\
      to the Licensor or its representatives, including but not limited to\n\
      communication on electronic mailing lists, source code control systems,\n\
      and issue tracking systems that are managed by, or on behalf of, the\n\
      Licensor for the purpose of discussing and improving the Work, but\n\
      excluding communication that is conspicuously marked or otherwise\n\
      designated in writing by the copyright owner as \"Not a Contribution.\"\n\
\n\
      \"Contributor\" shall mean Licensor and any individual or Legal Entity\n\
      on behalf of whom a Contribution has been received by Licensor and\n\
      subsequently incorporated within the Work.\n\
\n\
   2. Grant of Copyright License. Subject to the terms and conditions of\n\
      this License, each Contributor hereby grants to You a perpetual,\n\
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n\
      copyright license to reproduce, prepare Derivative Works of,\n\
      publicly display, publicly perform, sublicense, and distribute the\n\
      Work and such Derivative Works in Source or Object form.\n\
\n\
   3. Grant of Patent License. Subject to the terms and conditions of\n\
      this License, each Contributor hereby grants to You a perpetual,\n\
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n\
      (except as stated in this section) patent license to make, have made,\n\
      use, offer to sell, sell, import, and otherwise transfer the Work,\n\
      where such license applies only to those patent claims licensable\n\
      by such Contributor that are necessarily infringed by their\n\
      Contribution(s) alone or by combination of their Contribution(s)\n\
      with the Work to which such Contribution(s) was submitted. If You\n\
      institute patent litigation against any entity (including a\n\
      cross-claim or counterclaim in a lawsuit) alleging that the Work\n\
      or a Contribution incorporated within the Work constexpritutes direct\n\
      or contributory patent infringement, then any patent licenses\n\
      granted to You under this License for that Work shall terminate\n\
      as of the date such litigation is filed.\n\
\n\
   4. Redistribution. You may reproduce and distribute copies of the\n\
      Work or Derivative Works thereof in any medium, with or without\n\
      modifications, and in Source or Object form, provided that You\n\
      meet the following conditions:\n\
\n\
      (a) You must give any other recipients of the Work or\n\
          Derivative Works a copy of this License; and\n\
\n\
      (b) You must cause any modified files to carry prominent notices\n\
          stating that You changed the files; and\n\
\n\
      (c) You must retain, in the Source form of any Derivative Works\n\
          that You distribute, all copyright, patent, trademark, and\n\
          attribution notices from the Source form of the Work,\n\
          excluding those notices that do not pertain to any part of\n\
          the Derivative Works; and\n\
\n\
      (d) If the Work includes a \"NOTICE\" text file as part of its\n\
          distribution, then any Derivative Works that You distribute must\n\
          include a readable copy of the attribution notices contained\n\
          within such NOTICE file, excluding those notices that do not\n\
          pertain to any part of the Derivative Works, in at least one\n\
          of the following places: within a NOTICE text file distributed\n\
          as part of the Derivative Works; within the Source form or\n\
          documentation, if provided along with the Derivative Works; or,\n\
          within a display generated by the Derivative Works, if and\n\
          wherever such third-party notices normally appear. The contents\n\
          of the NOTICE file are for informational purposes only and\n\
          do not modify the License. You may add Your own attribution\n\
          notices within Derivative Works that You distribute, alongside\n\
          or as an addendum to the NOTICE text from the Work, provided\n\
          that such additional attribution notices cannot be constexprrued\n\
          as modifying the License.\n\
\n\
      You may add Your own copyright statement to Your modifications and\n\
      may provide additional or different license terms and conditions\n\
      for use, reproduction, or distribution of Your modifications, or\n\
      for any such Derivative Works as a whole, provided Your use,\n\
      reproduction, and distribution of the Work otherwise complies with\n\
      the conditions stated in this License.\n\
\n\
   5. Submission of Contributions. Unless You explicitly state otherwise,\n\
      any Contribution intentionally submitted for inclusion in the Work\n\
      by You to the Licensor shall be under the terms and conditions of\n\
      this License, without any additional terms or conditions.\n\
      Notwithstanding the above, nothing herein shall supersede or modify\n\
      the terms of any separate license agreement you may have executed\n\
      with Licensor regarding such Contributions.\n\
\n\
   6. Trademarks. This License does not grant permission to use the trade\n\
      names, trademarks, service marks, or product names of the Licensor,\n\
      except as required for reasonable and customary use in describing the\n\
      origin of the Work and reproducing the content of the NOTICE file.\n\
\n\
   7. Disclaimer of Warranty. Unless required by applicable law or\n\
      agreed to in writing, Licensor provides the Work (and each\n\
      Contributor provides its Contributions) on an \"AS IS\" BASIS,\n\
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or\n\
      implied, including, without limitation, any warranties or conditions\n\
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A\n\
      PARTICULAR PURPOSE. You are solely responsible for determining the\n\
      appropriateness of using or redistributing the Work and assume any\n\
      risks associated with Your exercise of permissions under this License.\n\
\n\
   8. Limitation of Liability. In no event and under no legal theory,\n\
      whether in tort (including negligence), contract, or otherwise,\n\
      unless required by applicable law (such as deliberate and grossly\n\
      negligent acts) or agreed to in writing, shall any Contributor be\n\
      liable to You for damages, including any direct, indirect, special,\n\
      incidental, or consequential damages of any character arising as a\n\
      result of this License or out of the use or inability to use the\n\
      Work (including but not limited to damages for loss of goodwill,\n\
      work stoppage, computer failure or malfunction, or any and all\n\
      other commercial damages or losses), even if such Contributor\n\
      has been advised of the possibility of such damages.\n\
\n\
   9. Accepting Warranty or Additional Liability. While redistributing\n\
      the Work or Derivative Works thereof, You may choose to offer,\n\
      and charge a fee for, acceptance of support, warranty, indemnity,\n\
      or other liability obligations and/or rights consistent with this\n\
      License. However, in accepting such obligations, You may act only\n\
      on Your own behalf and on Your sole responsibility, not on behalf\n\
      of any other Contributor, and only if You agree to indemnify,\n\
      defend, and hold each Contributor harmless for any liability\n\
      incurred by, or claims asserted against, such Contributor by reason\n\
      of your accepting any such warranty or additional liability.\n\
\n\
   END OF TERMS AND CONDITIONS";

constexpr char lib_mbedtls[]="Mbed TLS";
constexpr char license_mbedtls[]="\
                                 Apache License\n\
                           Version 2.0, January 2004\n\
                        http://www.apache.org/licenses/\n\
\n\
   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION\n\
\n\
   1. Definitions.\n\
\n\
      \"License\" shall mean the terms and conditions for use, reproduction,\n\
      and distribution as defined by Sections 1 through 9 of this document.\n\
\n\
      \"Licensor\" shall mean the copyright owner or entity authorized by\n\
      the copyright owner that is granting the License.\n\
\n\
      \"Legal Entity\" shall mean the union of the acting entity and all\n\
      other entities that control, are controlled by, or are under common\n\
      control with that entity. For the purposes of this definition,\n\
      \"control\" means (i) the power, direct or indirect, to cause the\n\
      direction or management of such entity, whether by contract or\n\
      otherwise, or (ii) ownership of fifty percent (50%) or more of the\n\
      outstanding shares, or (iii) beneficial ownership of such entity.\n\
\n\
      \"You\" (or \"Your\") shall mean an individual or Legal Entity\n\
      exercising permissions granted by this License.\n\
\n\
      \"Source\" form shall mean the preferred form for making modifications,\n\
      including but not limited to software source code, documentation\n\
      source, and configuration files.\n\
\n\
      \"Object\" form shall mean any form resulting from mechanical\n\
      transformation or translation of a Source form, including but\n\
      not limited to compiled object code, generated documentation,\n\
      and conversions to other media types.\n\
\n\
      \"Work\" shall mean the work of authorship, whether in Source or\n\
      Object form, made available under the License, as indicated by a\n\
      copyright notice that is included in or attached to the work\n\
      (an example is provided in the Appendix below).\n\
\n\
      \"Derivative Works\" shall mean any work, whether in Source or Object\n\
      form, that is based on (or derived from) the Work and for which the\n\
      editorial revisions, annotations, elaborations, or other modifications\n\
      represent, as a whole, an original work of authorship. For the purposes\n\
      of this License, Derivative Works shall not include works that remain\n\
      separable from, or merely link (or bind by name) to the interfaces of,\n\
      the Work and Derivative Works thereof.\n\
\n\
      \"Contribution\" shall mean any work of authorship, including\n\
      the original version of the Work and any modifications or additions\n\
      to that Work or Derivative Works thereof, that is intentionally\n\
      submitted to Licensor for inclusion in the Work by the copyright owner\n\
      or by an individual or Legal Entity authorized to submit on behalf of\n\
      the copyright owner. For the purposes of this definition, \"submitted\"\n\
      means any form of electronic, verbal, or written communication sent\n\
      to the Licensor or its representatives, including but not limited to\n\
      communication on electronic mailing lists, source code control systems,\n\
      and issue tracking systems that are managed by, or on behalf of, the\n\
      Licensor for the purpose of discussing and improving the Work, but\n\
      excluding communication that is conspicuously marked or otherwise\n\
      designated in writing by the copyright owner as \"Not a Contribution.\"\n\
\n\
      \"Contributor\" shall mean Licensor and any individual or Legal Entity\n\
      on behalf of whom a Contribution has been received by Licensor and\n\
      subsequently incorporated within the Work.\n\
\n\
   2. Grant of Copyright License. Subject to the terms and conditions of\n\
      this License, each Contributor hereby grants to You a perpetual,\n\
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n\
      copyright license to reproduce, prepare Derivative Works of,\n\
      publicly display, publicly perform, sublicense, and distribute the\n\
      Work and such Derivative Works in Source or Object form.\n\
\n\
   3. Grant of Patent License. Subject to the terms and conditions of\n\
      this License, each Contributor hereby grants to You a perpetual,\n\
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable\n\
      (except as stated in this section) patent license to make, have made,\n\
      use, offer to sell, sell, import, and otherwise transfer the Work,\n\
      where such license applies only to those patent claims licensable\n\
      by such Contributor that are necessarily infringed by their\n\
      Contribution(s) alone or by combination of their Contribution(s)\n\
      with the Work to which such Contribution(s) was submitted. If You\n\
      institute patent litigation against any entity (including a\n\
      cross-claim or counterclaim in a lawsuit) alleging that the Work\n\
      or a Contribution incorporated within the Work constexpritutes direct\n\
      or contributory patent infringement, then any patent licenses\n\
      granted to You under this License for that Work shall terminate\n\
      as of the date such litigation is filed.\n\
\n\
   4. Redistribution. You may reproduce and distribute copies of the\n\
      Work or Derivative Works thereof in any medium, with or without\n\
      modifications, and in Source or Object form, provided that You\n\
      meet the following conditions:\n\
\n\
      (a) You must give any other recipients of the Work or\n\
          Derivative Works a copy of this License; and\n\
\n\
      (b) You must cause any modified files to carry prominent notices\n\
          stating that You changed the files; and\n\
\n\
      (c) You must retain, in the Source form of any Derivative Works\n\
          that You distribute, all copyright, patent, trademark, and\n\
          attribution notices from the Source form of the Work,\n\
          excluding those notices that do not pertain to any part of\n\
          the Derivative Works; and\n\
\n\
      (d) If the Work includes a \"NOTICE\" text file as part of its\n\
          distribution, then any Derivative Works that You distribute must\n\
          include a readable copy of the attribution notices contained\n\
          within such NOTICE file, excluding those notices that do not\n\
          pertain to any part of the Derivative Works, in at least one\n\
          of the following places: within a NOTICE text file distributed\n\
          as part of the Derivative Works; within the Source form or\n\
          documentation, if provided along with the Derivative Works; or,\n\
          within a display generated by the Derivative Works, if and\n\
          wherever such third-party notices normally appear. The contents\n\
          of the NOTICE file are for informational purposes only and\n\
          do not modify the License. You may add Your own attribution\n\
          notices within Derivative Works that You distribute, alongside\n\
          or as an addendum to the NOTICE text from the Work, provided\n\
          that such additional attribution notices cannot be constexprrued\n\
          as modifying the License.\n\
\n\
      You may add Your own copyright statement to Your modifications and\n\
      may provide additional or different license terms and conditions\n\
      for use, reproduction, or distribution of Your modifications, or\n\
      for any such Derivative Works as a whole, provided Your use,\n\
      reproduction, and distribution of the Work otherwise complies with\n\
      the conditions stated in this License.\n\
\n\
   5. Submission of Contributions. Unless You explicitly state otherwise,\n\
      any Contribution intentionally submitted for inclusion in the Work\n\
      by You to the Licensor shall be under the terms and conditions of\n\
      this License, without any additional terms or conditions.\n\
      Notwithstanding the above, nothing herein shall supersede or modify\n\
      the terms of any separate license agreement you may have executed\n\
      with Licensor regarding such Contributions.\n\
\n\
   6. Trademarks. This License does not grant permission to use the trade\n\
      names, trademarks, service marks, or product names of the Licensor,\n\
      except as required for reasonable and customary use in describing the\n\
      origin of the Work and reproducing the content of the NOTICE file.\n\
\n\
   7. Disclaimer of Warranty. Unless required by applicable law or\n\
      agreed to in writing, Licensor provides the Work (and each\n\
      Contributor provides its Contributions) on an \"AS IS\" BASIS,\n\
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or\n\
      implied, including, without limitation, any warranties or conditions\n\
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A\n\
      PARTICULAR PURPOSE. You are solely responsible for determining the\n\
      appropriateness of using or redistributing the Work and assume any\n\
      risks associated with Your exercise of permissions under this License.\n\
\n\
   8. Limitation of Liability. In no event and under no legal theory,\n\
      whether in tort (including negligence), contract, or otherwise,\n\
      unless required by applicable law (such as deliberate and grossly\n\
      negligent acts) or agreed to in writing, shall any Contributor be\n\
      liable to You for damages, including any direct, indirect, special,\n\
      incidental, or consequential damages of any character arising as a\n\
      result of this License or out of the use or inability to use the\n\
      Work (including but not limited to damages for loss of goodwill,\n\
      work stoppage, computer failure or malfunction, or any and all\n\
      other commercial damages or losses), even if such Contributor\n\
      has been advised of the possibility of such damages.\n\
\n\
   9. Accepting Warranty or Additional Liability. While redistributing\n\
      the Work or Derivative Works thereof, You may choose to offer,\n\
      and charge a fee for, acceptance of support, warranty, indemnity,\n\
      or other liability obligations and/or rights consistent with this\n\
      License. However, in accepting such obligations, You may act only\n\
      on Your own behalf and on Your sole responsibility, not on behalf\n\
      of any other Contributor, and only if You agree to indemnify,\n\
      defend, and hold each Contributor harmless for any liability\n\
      incurred by, or claims asserted against, such Contributor by reason\n\
      of your accepting any such warranty or additional liability.\n\
\n\
   END OF TERMS AND CONDITIONS";

#endif