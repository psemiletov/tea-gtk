import glob
import os


env = Environment(CC = 'gcc', tools = ['default', 'gettext'])

TEA_BIN_NAME = 'teagtk'

INST_PREFIX = '/usr/local/'
INST_DIR_BIN = INST_PREFIX + 'bin'

INST_PREFIX_DATA = INST_PREFIX + 'share'
INST_DIR_DATA = INST_PREFIX_DATA + '/teagtk' 

TEA_DOC_DIR = INST_DIR_DATA + '/doc' 
TEA_IMAGES_DIR = INST_DIR_DATA + '/images' 


TEA_TRANS_DIR = INST_PREFIX_DATA + '/locale'

SOURCES = glob.glob('./src/*.c')


def create_uninstall_target(env, path, is_glob):
    if is_glob:
        all_files = Glob(path,strings=True)
        for filei in all_files:
            env.Command( "uninstall-"+filei, filei,
            [
            Delete("$SOURCE"),
            ])
            env.Alias("uninstall", "uninstall-"+filei)   
    else:
        env.Command( "uninstall-"+path, path,
        [
        Delete("$SOURCE"),
        ])
        env.Alias("uninstall", "uninstall-"+path) 


if 'uninstall' in COMMAND_LINE_TARGETS:
    # create uninstall targets
    create_uninstall_target(env, INST_DIR_BIN + '/' + TEA_BIN_NAME, False)
    create_uninstall_target(env, INST_DIR_DATA + '/*', True)
    create_uninstall_target(env, TEA_TRANS_DIR + "/*/LC_MESSAGES/teagtk.mo", True)

   # create_uninstall_target(env, "/usr/local/share/locale/*/LC_MESSAGES/myapp.mo", True)

#if 'uninstall' in COMMAND_LINE_TARGETS:
#   env.Command("uninstall", None, Delete(FindInstalledFiles()))





#SConscript('./src/po/SConscript.i18n', exports = 'env')
SConscript('./src/po/SConscript', exports = 'env')







env.ParseConfig('pkg-config --cflags --libs gtk+-3.0')
env.ParseConfig('pkg-config --cflags --libs gtksourceview-3.0')
#env.ParseConfig('pkg-config --cflags --libs libcurl')
#env.ParseConfig('pkg-config --cflags --libs zziplib')


conf = Configure(env)

if conf.CheckLib('gtk-3'):
   print ('GTK+3 has been found')

#if conf.CheckLib('zziplib'):
if conf.CheckCHeader('zziplib.h'):
  env.Append(CPPDEFINES = [ ('ZZIPLIB_SUPPORTED', 1) ])
  env.ParseConfig('pkg-config --cflags --libs zziplib')

if conf.CheckCHeader('aspell.h'):
   env.Append(CPPDEFINES = [ ('HAVE_LIBASPELL', 1) ])
   env.MergeFlags('-laspell')
  

TEADOCS_SOURCE_DIR = glob.glob('./doc/*')
TEAIMAGES_SOURCE_DIR = glob.glob('./images/*')


env.Append(CFLAGS = ['-w'])
env.Append(CFLAGS = ['-g'])


env.Append(CPPDEFINES = [ ('VERSION', '\\"18.0.0\\"') ])


env.Append(CPPDEFINES = [ ('TEA_DOC_DIR', '\\"' + TEA_DOC_DIR + '\\"') ])
env.Append(CPPDEFINES = [ ('PACKAGE_DATA_DIR', '\\"' + INST_DIR_DATA + '\\"') ])
env.Append(CPPDEFINES = [ ('TEA_TRANS_DIR', '\\"' + TEA_TRANS_DIR + '\\"') ])
env.Append(CPPDEFINES = [ ('GETTEXT_PACKAGE', '\\"' + TEA_BIN_NAME + '\\"') ])





#env.Append(LIBS = ['gtk+-3.0', 'gtksourceview-3.0', 'libcurl', 'zziplib'])

teagtkbin = env.Program(target = 'teagtk', source = SOURCES)

env.MergeFlags('-DNIX=1 -lm')




env.Install(dir = INST_DIR_BIN, source = teagtkbin)

#env.Alias('install', [INST_DIR_BIN, INST_DIR_DATA])


env.Alias('install', [INST_DIR_BIN])

env.Alias('install', InstallAs([TEA_TRANS_DIR + "/cs/LC_MESSAGES/teagtk.mo"], ["./src/po/cs.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/de/LC_MESSAGES/teagtk.mo"], ["./src/po/de.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/es/LC_MESSAGES/teagtk.mo"], ["./src/po/es.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/fr/LC_MESSAGES/teagtk.mo"], ["./src/po/fr.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/ja/LC_MESSAGES/teagtk.mo"], ["./src/po/ja.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/lv/LC_MESSAGES/teagtk.mo"], ["./src/po/lv.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/pl/LC_MESSAGES/teagtk.mo"], ["./src/po/pl.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/ru/LC_MESSAGES/teagtk.mo"], ["./src/po/ru.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/sr/LC_MESSAGES/teagtk.mo"], ["./src/po/sr.mo"]))
env.Alias('install', InstallAs([TEA_TRANS_DIR + "/uk/LC_MESSAGES/teagtk.mo"], ["./src/po/uk.mo"]))


files = Glob('./doc/*')
data = env.Install(TEA_DOC_DIR, files)
env.Alias('install', data)

env.Alias('install', InstallAs([TEA_DOC_DIR + "/AUTHORS"], ["./AUTHORS"]))
env.Alias('install', InstallAs([TEA_DOC_DIR + "/ChangeLog"], ["./ChangeLog"]))
env.Alias('install', InstallAs([TEA_DOC_DIR + "/COPYING"], ["./COPYING"]))
env.Alias('install', InstallAs([TEA_DOC_DIR + "/NEWS"], ["./NEWS"]))
env.Alias('install', InstallAs([TEA_DOC_DIR + "/README"], ["./README"]))
env.Alias('install', InstallAs([TEA_DOC_DIR + "/TODO"], ["./TODO"]))

files = Glob('./images/*')
data = env.Install(TEA_IMAGES_DIR, files)
env.Alias('install', data)

##################
#env.Alias('install', InstallAs([TEA_IMAGES_DIR + "/tea_logo.jpg"], ["./images/tea_logo.jpg"]))
#env.Alias('install', InstallAs([TEA_IMAGES_DIR + "/tea_icon.png"], ["./images/tea_icon.png"]))


