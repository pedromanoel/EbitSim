SM_SOURCE=DownloadSM.sm UploadSM.sm ConnectionSM.sm
#SM_GRAPHIC=StartUp.sm
CC_DIR=out
IMG_DIR=images
#SM_SOURCE=PeerWireSM.sm
SMC=~/bin/smc
#SMC=/Users/pevangelista/bin/smc
SMC_FLAGS=-c++ -sufix cc -g
RM_F=rm -f

all: ${SM_SOURCE:.sm=_sm.cc}

graph : ${SM_SOURCE:%.sm=${IMG_DIR}/%_sm.dot} ${SM_GRAPHIC:%.sm=${IMG_DIR}/%_sm.dot}

png : ${SM_SOURCE:%.sm=${IMG_DIR}/%_sm.png} ${SM_GRAPHIC:%.sm=${IMG_DIR}/%_sm.png}

table : ${SM_SOURCE:%.sm=${IMG_DIR}/%_sm.html} ${SM_GRAPHIC:%.sm=${IMG_DIR}/%_sm.html}

.PHONY: clean
clean:
		-${RM_F} ${CC_DIR}/*_sm.h
		-${RM_F} ${CC_DIR}/*_sm.cc

.PHONY: clean_img
clean_img:
		-${RM_F} ${IMG_DIR}/*_sm.dot
		-${RM_F} ${IMG_DIR}/*_sm.png
		-${RM_F} ${IMG_DIR}/*_sm.html

%_sm.h %_sm.cc: %.sm
		${SMC} -d ${CC_DIR} -hearderd ${CC_DIR} ${SMC_FLAGS} $<
		./refactor.py $< ${CC_DIR}/$@ 

${IMG_DIR}/%_sm.png: ${IMG_DIR}/%_sm.dot
		dot -T png -o $@ $<

${IMG_DIR}/%_sm.dot: %.sm
		${SMC} -d ${IMG_DIR} -graph -glevel 2 $<

${IMG_DIR}/%_sm.html: %.sm
		${SMC} -d ${IMG_DIR} -table $<