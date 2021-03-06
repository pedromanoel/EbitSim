#!/usr/bin/python
# EbitSim - Enhanced BitTorrent Simulation
# This program is under the terms of the Creative Commons
# Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
#
# You are free:
#
#    to Share - to copy, distribute and transmit the work
#    to Remix - to adapt the work
#
# Under the following conditions:
#
#    Attribution - You must attribute the work in the manner specified by the
#    author or licensor (but not in any way that suggests that they endorse you
#    or your use of the work).
#
#    Noncommercial - You may not use this work for commercial purposes.
#
#    Share Alike - If you alter, transform, or build upon this work, you may
#    distribute the resulting work only under the same or similar license to
#    this one.
#
# With the understanding that:
#
#    Waiver - Any of the above conditions can be waived if you get permission
#    from the copyright holder.
#
#    Public Domain - Where the work or any of its elements is in the public
#    domain under applicable law, that status is in no way affected by the
#    license.
#
#    Other Rights - In no way are any of the following rights affected by the
#    license:
#        - Your fair dealing or fair use rights, or other applicable copyright
#          exceptions and limitations;
#        - The author's moral rights;
#        - Rights other persons may have either in the work itself or in how
#          the work is used, such as publicity or privacy rights.
#
#    Notice - For any reuse or distribution, you must make clear to others the
#    license terms of this work. The best way to do this is with a link to this
#    web page. <http:#creativecommons.org/licenses/by-nc-sa/3.0/>
#
# Author:
#     Pedro Manoel Fabiano Alves Evangelista <pevangelista@larc.usp.br>
#     Supervised by Prof Tereza Cristina M. B. Carvalho <carvalho@larc.usp.br>
#     Graduate Student at Escola Politecnica of University of Sao Paulo, Brazil
#
# Contributors:
#     Marcelo Carneiro do Amaral <mamaral@larc.usp.br>
#     Victor Souza <victor.souza@ericsson.com>
#
# Disclaimer:
#     This work is part of a Master Thesis developed by:
#        Pedro Evangelista, graduate student at
#        Laboratory of Computer Networks and Architecture
#        Escola Politecnica
#        University of Sao Paulo
#        Brazil
#     and supported by:
#        Innovation Center
#        Ericsson Telecomunicacoes S.A., Brazil.
#
# UNLESS OTHERWISE MUTUALLY AGREED TO BY THE PARTIES IN WRITING AND TO THE
# FULLEST EXTENT PERMITTED BY APPLICABLE LAW, LICENSOR OFFERS THE WORK AS-IS
# AND MAKES NO REPRESENTATIONS OR WARRANTIES OF ANY KIND CONCERNING THE WORK,
# EXPRESS, IMPLIED, STATUTORY OR OTHERWISE, INCLUDING, WITHOUT LIMITATION,
# WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# NONINFRINGEMENT, OR THE ABSENCE OF LATENT OR OTHER DEFECTS, ACCURACY, OR THE
# PRESENCE OF ABSENCE OF ERRORS, WHETHER OR NOT DISCOVERABLE. SOME
# JURISDICTIONS DO NOT ALLOW THE EXCLUSION OF IMPLIED WARRANTIES, SO THIS
# EXCLUSION MAY NOT APPLY TO YOU.
#
def refactor_file(class_name, source_file):
    file_content = ""
    with open(source_file) as f:
        file_content = f.read()

    file_content = file_content.replace("PeerWireThreadState", class_name + "State")

    with open(source_file, "w") as f:
        f.write(file_content)

def show_usage():
    print "Usage:", sys.argv[0], "<class_name.sm> <source_file.cc>"
    print "Refactor the class PeerWireThreadState in the <source file>."

if __name__ == "__main__":
    import sys

    if len(sys.argv) != 3:
        print "Wrong number of arguments"
        show_usage()
#        for i, par in enumerate(sys.argv):
#            print i, par
    else:
        class_name, sep, class_ext = sys.argv[1].partition(".")
        source_name, sep, source_ext = sys.argv[2].partition(".")
        if class_ext and source_ext:
            refactor_file(class_name, source_name + ".cc")
            refactor_file(class_name, source_name + ".h")
        else:
            print "Invalid arguments"
            show_usage()
