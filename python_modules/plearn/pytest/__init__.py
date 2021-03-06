__version_id__ = "$Id: __init__.py 3647 2005-06-23 15:49:51Z dorionc $"
                            
import os, sys, time, logging

from plearn.utilities import toolkit
from plearn.utilities.ModeAndOptionParser import Mode, ModeAndOptionParser, OptionGroup

# PyTest Modules
import core, modes 

__all__ = [ "main" ]
    
###################################################################################
## MAIN PROGRAM
def main( pytest_version ):
    os.environ['PYTEST_STATE'] = 'Active'
 
    parser = ModeAndOptionParser( usage = "%prog mode [options] target*",
                                  version = "%prog " + pytest_version()   )
    
    verb_levels = [l for l in logging._levelNames if isinstance(l, basestring)]
    parser.add_option( '-v', "--verbosity",
                       choices=verb_levels, default="INFO",
                       help="One of %s [defaults to INFO]"%','.join(verb_levels) )
    
    parser.add_option("--mail", default=None,
                      help='Not supported yet.')
    
    parser.add_option( '--traceback',
                       action="store_true",
                       help="This flag triggers routines to report the traceback of "
                       "PyTestUsageError or KeyboardInterrupt. By default, "
                       "only the test's name and message are reported.",
                       default = False )

    parser.add_option( '--debug',
                       default=False, action="store_true", 
                       help="--debug is equivalent to --traceback --verbosity DEBUG.")

    try:
        options, targets   = parser.parse_args()
        if options.debug:
            options.traceback = True
            options.verbosity = "DEBUG"
    except SystemExit, e:
        core.exitPyTest("USAGE ERROR")
    
    ############################################################
    ################## Some preprocessing ######################
    
    ## Managing the verbosity option.
    # In Python2.4: logging.basicConfig(level=logging.DEBUG, ...)
    core.hdlr = logging.StreamHandler()
    core.hdlr.setFormatter( logging.Formatter("%(message)s") )
    logging.root.addHandler(core.hdlr)
    logging.root.setLevel(logging._levelNames[options.verbosity])
    
    if ( hasattr( options, 'mail' )
         and options.mail is not None ):
        raise NotImplementedError
    
    
    ############################################################
    # Launching the selected mode: The main part of the program
    ############################################################
    
    ## Program name and copyrights with version number
    newline = '\n  '
    logging.info( newline.join(
        [ "%sPyTest %s"%(newline,pytest_version()),
          "(c) 2004-2006, Christian Dorion",
          "This is free software distributed under a BSD type license.",
          "Report problems to dorionc@apstat.com\n" ]) )
    
    try:
        parser.selected_mode( targets, options )    

    except KeyboardInterrupt, kex:
        if options.traceback:
            print core.traceback(kex)
        else:            
            logging.info("Interupted by user.")

    except core.PyTestUsageError, e: 
        core.updateExitCode("USAGE ERROR")
        if options.traceback:
            print core.traceback(e)
        else:
            logging.critical( "%s: %s." % (e.__class__.__name__,e) )

    except Exception, unexpected: 
        core.updateExitCode("INTERNAL ERROR")
        print core.traceback(unexpected)

    logging.debug("\nQuitting PyTest.")
    logging.info('')
    core.exitPyTest()
