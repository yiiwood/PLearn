#
# options_dialog
# Copyright (C) 2007 Xavier Saint-Mleux, ApSTAT Technologies inc.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

import sys, os

def getVerbosity(args):
    """
    get verbosity from PLearn
    """
    verb_pos= -1
    for i in range(len(args)):
        if args[i].startswith('--verbosity='):
            verb_pos= i
    verb= 5
    if verb_pos >= 0:
        verb= int(args[verb_pos].split('=')[1])
    return verb, verb_pos

def getModulesToLog(args):
    """
    get modules to log
    """
    logs_pos= -1
    for i in range(len(args)):
        if args[i].startswith('--enable-logging='):
            logs_pos= i
    logs= ['__NONE__']
    if logs_pos >= 0:
        logs= args[logs_pos].split('=')[1].split(',')
    return logs, logs_pos

def getGuiNamespaces(args):
    gui_pos= -1
    # get +gui option
    for i in range(len(args)):
        if args[i].startswith('+gui'):
            gui_pos= i
    gui_namespaces= ['__ALL__']
    if gui_pos >= 0:
        x= args[gui_pos].split(':')
        if len(x)>1:
            gui_namespaces= x[1].split(',')
    return gui_namespaces, gui_pos

def getGuiInfo(args):
    """
    get verbosity, modules to log and
    namespaces from command line
    """
    verb, pos= getVerbosity(args)
    if pos >= 0: del args[pos]
    logs, pos= getModulesToLog(args)
    if pos >= 0: del args[pos]
    namespaces, pos= getGuiNamespaces(args)
    if pos >= 0: del args[pos]
    return verb, logs, namespaces, pos>=0

def gladeFile():
    #import plearn.plide.plide
    import plearn
    return os.path.join(os.path.dirname(plearn.utilities.options_dialog.__file__),
                        "resources", "options_dialog.glade")

def optionsDialog(name, expdir, verbosity, named_logging, namespaces):
    """
    pop a dialog showing all plnamespaces
    """
    from plearn.plide import plide_options
    import gtk
    plide_options.PyPLearnOptionsDialog.define_injected(None, gladeFile)
    options_holder= plide_options.PyPLearnOptionsHolder(name, None,
                                                        expdir, namespaces)
    options_holder.log_enable= named_logging
    ks= options_holder.inv_verb_map.keys()
    ks.sort()
    for k in ks:
        if k <= verbosity:
            options_holder.log_verbosity= options_holder.inv_verb_map[k]
    options_dialog= plide_options.PyPLearnOptionsDialog(options_holder)
    result= options_dialog.run()
    if result == gtk.RESPONSE_OK:
       options_dialog.update_options_holder()

    # Because we are not running under the Gtk main loop, we need to call
    # the Gtk main loop manually a couple of times to allow it to clean up
    # and remove the window. 
    for i in range(5):
        gtk.main_iteration()
    
    return result == gtk.RESPONSE_OK, \
           options_holder.verbosity_map.get(options_holder.log_verbosity, 5), \
           options_holder.log_enable

def simpleOptionsDialog(plnamespaces):
    """Pops a dialog showing only the given plnamespaces. No extra
    tabs (expdir, verbosity, manual overrides) added.

    @param plnamespaces   A list of strings naming the plnamespace subclasses.
    @return True if the user clicked ok, false otherwise."""
    
    from plearn.plide.plide_options import PyPLearnOptionsDialog, PyPLearnOptionsHolder
    import gtk
    
    PyPLearnOptionsDialog.define_injected(None, gladeFile)
    options_holder = PyPLearnOptionsHolder('', None, '', plnamespaces)
    
    options_dialog = PyPLearnOptionsDialog(options_holder,
                                           include_standard_script_options=False)
    result = options_dialog.run()
    if result == gtk.RESPONSE_OK:
        options_dialog.update_options_holder()

    # Because we are not running under the Gtk main loop, we need to call
    # the Gtk main loop manually a couple of times to allow it to clean up
    # and remove the window. 
    for i in range(5):
        gtk.main_iteration()

    return result == gtk.RESPONSE_OK
