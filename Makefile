git = git -c user.name="Auto" -c user.email="auto@auto.com" 

BUILDDIR = $(PWD)/build
SRCDIR = $(PWD)/src
LLVM_VERSION=4.0.0

all : release

.PHONY : release debug run clean patch test

release : $(BUILDDIR)/buildr/Makefile $(BUILDDIR)/depqbf/depqbf
	+make -C $(BUILDDIR)/buildr
	cp -f $(BUILDDIR)/buildr/vts-synth vts-synth

debug : $(BUILDDIR)/buildd/Makefile $(BUILDDIR)/depqbf/depqbf
	+make -C $(BUILDDIR)/buildd
	cp -f $(BUILDDIR)/buildd/vts-synth vts-synth

$(BUILDDIR)/buildr/Makefile: $(BUILDDIR)/z3/buildr/libz3.so
	mkdir -p $(BUILDDIR)/buildr
	cd $(BUILDDIR)/buildr; cmake -DCMAKE_BUILD_TYPE=Release $(SRCDIR)

$(BUILDDIR)/buildd/Makefile: $(BUILDDIR)/z3/buildd/libz3.so
	mkdir -p $(BUILDDIR)/buildd
	cd $(BUILDDIR)/buildd; cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_VERSION=$(LLVM_VERSION) -DZ3_DEBUG:BOOL=TRUE $(SRCDIR)

clean :
	rm -rf $(BUILDDIR)/buildr
	rm -rf $(BUILDDIR)/buildd
	rm -f vts-synth
	find -name "*~"| xargs rm -rf


#-----------------------------------------------------------------------------
# Z3 fetch and compile


patch :
	mkdir -p $(SRCDIR)/solver/z3-patch
	cd $(BUILDDIR)/z3; $(git) diff > $(SRCDIR)/solver/z3-patch/z3.patch


# NEW_Z3_FILES =  $(SRCDIR)/z3-patch/smt_model_reporter.cpp \
# 		$(SRCDIR)/z3-patch/special_relations_decl_plugin.cpp \
# 		$(SRCDIR)/z3-patch/special_relations_decl_plugin.h \
# 		$(SRCDIR)/z3-patch/theory_special_relations.cpp \
# 		$(SRCDIR)/z3-patch/theory_special_relations.h \
# 		$(SRCDIR)/z3-patch/api_special_relations.cpp


$(BUILDDIR)/z3/patched : $(SRCDIR)/solver/z3-patch/z3.patch $(BUILDDIR)/z3/README.md
	cd $(BUILDDIR)/z3; $(git) stash clear && $(git) stash save && $(git) apply --whitespace=fix $(SRCDIR)/solver/z3-patch/z3.patch
	touch $(BUILDDIR)/z3/patched

# $(BUILDDIR)/z3/newfiles : $(NEW_Z3_FILES) $(BUILDDIR)/z3/README.md
# 	cd $(BUILDDIR)/z3/src/smt; ln -sf $(SRCDIR)/z3-patch/smt_model_reporter.cpp smt_model_reporter.cpp; ln -sf $(SRCDIR)/z3-patch/theory_special_relations.cpp theory_special_relations.cpp; ln -sf $(SRCDIR)/z3-patch/theory_special_relations.h theory_special_relations.h
# 	cd $(BUILDDIR)/z3/src/ast; ln -sf $(SRCDIR)/z3-patch/special_relations_decl_plugin.cpp special_relations_decl_plugin.cpp; ln -sf $(SRCDIR)/z3-patch/special_relations_decl_plugin.h special_relations_decl_plugin.h
# 	cd $(BUILDDIR)/z3/src/api; ln -sf $(SRCDIR)/z3-patch/api_special_relations.cpp api_special_relations.cpp
# 	touch $(BUILDDIR)/z3/newfiles

# $(BUILDDIR)/z3/buildr/Makefile: $(BUILDDIR)/z3/patched
# 	rm -rf $(BUILDDIR)/z3/buildr
# 	cd $(BUILDDIR)/z3; python scripts/mk_make.py --staticlib -b buildr

# $(BUILDDIR)/z3/buildr/libz3.so : $(BUILDDIR)/z3/buildr/Makefile $(BUILDDIR)/z3/newfiles
# 	+make -C $(BUILDDIR)/z3/buildr

# $(BUILDDIR)/z3/buildr/libz3.a : $(BUILDDIR)/z3/buildr/libz3.so

# $(BUILDDIR)/z3/buildd/Makefile: $(BUILDDIR)/z3/patched
# 	rm -rf $(BUILDDIR)/z3/buildd
# 	cd $(BUILDDIR)/z3; python scripts/mk_make.py --staticlib -d -b buildd

# $(BUILDDIR)/z3/buildd/libz3.so : $(BUILDDIR)/z3/buildd/Makefile $(BUILDDIR)/z3/newfiles
# 	+make -C $(BUILDDIR)/z3/buildd


$(BUILDDIR)/z3/README.md : 
	mkdir -p $(BUILDDIR)
	cd $(BUILDDIR);$(git) clone git@github.com:Z3Prover/z3.git
	cd $(BUILDDIR)/z3;$(git) checkout f17ead21f92ca2c48b4e8478cb752ecfca5da78e


$(BUILDDIR)/z3/buildr/Makefile: $(BUILDDIR)/z3/patched
	rm -rf $(BUILDDIR)/z3/buildr
	mkdir -p $(BUILDDIR)/z3/buildr
	cd $(BUILDDIR)/z3/buildr; cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../

$(BUILDDIR)/z3/buildr/libz3.so : $(BUILDDIR)/z3/buildr/Makefile
	+make -C $(BUILDDIR)/z3/buildr

$(BUILDDIR)/z3/buildr/libz3.a : $(BUILDDIR)/z3/buildr/libz3.so

$(BUILDDIR)/z3/buildd/Makefile: $(BUILDDIR)/z3/patched
	rm -rf $(BUILDDIR)/z3/buildd
	mkdir -p $(BUILDDIR)/z3/buildd
	cd $(BUILDDIR)/z3/buildd; cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug ../

$(BUILDDIR)/z3/buildd/libz3.so : $(BUILDDIR)/z3/buildd/Makefile
	+make -C $(BUILDDIR)/z3/buildd


$(BUILDDIR)/depqbf/depqbf : 
	cd $(BUILDDIR);$(git) clone git@github.com:lonsing/depqbf.git	
	cd $(BUILDDIR)/depqbf;./compile.sh

#---------------------------------------------------------------------------
# runtest:
# 	make -C examples/tester/

# test: release runtest

# to run in sugar synthesis mode
run:
	./vts-synth -r -i examples/ex2.sugar 

# ./vts-synth -r -i examples/ex1.sugar 

# features to be added
# -- add input syntax
# -- implement do not care?
# -- encode to allow runaway reactions 
# -- fix universal encode
# -- input language
