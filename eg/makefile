CFLAGS += -std=c99
CFLAGS += `pkg-config --cflags --libs goptfoo jkiss`
CFLAGS += -lpthread
# for Mac OS X set this remove jkiss from above and fiddle with
# other flags if necessary if libjkiss is in a different directory
# than goptfoo
#CFLAGS += -ljkiss

bias: bias.c

customseed: customseed.c

threadrate: threadrate.c

uniform-bias: uniform-bias.c

clean:
        @-rm -f bias customseed threadrate uniform-bias 2>/dev/null
