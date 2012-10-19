#ifndef nativeaudionode_h__
#define nativeaudionode_h__

#include "NativeAudioParameters.h"

#define NATIVE_AUDIONODE_ARGS_SIZE 32
#define NATIVE_AUDIONODE_CHANNEL_SIZE 32
#define NATIVE_AUDIONODE_WIRE_SIZE 32

#if 0
  #define SPAM(a) printf a
#else
  #define SPAM(a) (void)0
#endif

class NativeAudioNode
{
    public :
        enum TypeIO {INPUT, OUTPUT};
        enum ArgType {INT, DOUBLE};
        struct ExportsArgs {
            const char *name;
            ArgType type;
            void *ptr;

            ExportsArgs(const char *name, ArgType type, void *prt) : name(name), type(type), ptr(ptr) {};
        };
        ExportsArgs *args[NATIVE_AUDIONODE_ARGS_SIZE];

        NativeAudioNode(int inCount, int outCount, NativeAudioParameters *params, NativeSharedMessages *sharedMsg)
            : nodeProcessed(false), inQueueCount(0), inCount(inCount), outCount(outCount), params(params), sharedMsg(sharedMsg);
        {
            SPAM(("NativeAudioNode init located at %p\n", this));
            int max;

            // Init exports array
            for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
                this->args[i] = NULL;
            }

            // Init node IO queue
            for (int i = 0; i < inCount; i++) {
                this->input[i] = new NodeLink(INPUT, i, this);
            }

            for (int i = 0; i < outCount; i++) {
                this->output[i] = new NodeLink(OUTPUT, i, this);
            }

            // Malloc node I/O frames
            max = (inCount > outCount ? inCount : outCount);
            this->frames = (float **)calloc(sizeof(float), max);

            for (int i = 0; i < max; i++) {
                this->frames[i] = NULL;
            }

            // More output than input
            // malloc() the frames
            if (outCount > inCount || inCount == 0) {
                int s = inCount == 0 ? 0 : outCount - inCount;
                for (int i = 0; i < outCount; i++) {
                    printf("alloc buffer #%d\n", i);
                    if (i >= s) {
                        printf("=>malloc\n");
                        this->frames[i] = (float *)calloc(sizeof(float), this->params->bufferSize/this->params->channels);
                    }
                }
            }
        }

        struct NodeIO {
            NativeAudioNode *node;
            bool feedback;
            float *frame;
            NodeIO(NativeAudioNode *node, float *frame) : node(node), feedback(false), frame(frame) {};
        };

        struct NodeLink {
            int count;
            int channel;
            bool haveFeedback;
            NativeAudioNode *node;
            NodeIO *wire[32];
            TypeIO type;

            NodeLink (TypeIO type, int channel, NativeAudioNode *node) : 
                count(0), channel(channel), haveFeedback(false), node(node), type(type) 
            {
                for (int i = 0; i < 32; i++) {
                    wire[i] = NULL;
                }
            };
        };

        float **frames;

        bool nodeProcessed;
        NodeLink *input[32];
        NodeLink *output[32];

        int inQueueCount;

        int inCount;
        int outCount;

        void get(const char *name);
        bool set(const char *name, void *value, ArgType type) 
        {
            for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
                if (this->args[i] != NULL && strcmp(name, this->args[i]->name) == 0) {
                    if (this->args[i]->type == type) {
                        // Post to SharedMessage
                    } /* else {
                         return false;
                    } */
                }
            }

            return false;
        }

        // Check if output is connected to previous input
        void updateFeedback(NativeAudioNode *nOut) {
            SPAM(("updateFeedback called\n"));
            for (int i = 0; i < this->inCount; i++) {
                for (int j = 0; j < this->input[i]->count; j++) {
                    SPAM(("  checking input #%d wire %d; node = %p/%p\n", i, j, this->input[i]->wire[j]->node, nOut));
                    if (!this->input[i]->wire[j]->feedback && 
                         this->input[i]->wire[j]->node == nOut) {
                        SPAM(("=================== Its a feedback\n"));
                        // It's a feedback
                        this->input[i]->wire[j]->feedback = true;
                        this->input[i]->haveFeedback = true;
                        return;
                    } else if (!this->input[i]->wire[j]->feedback) {
                        SPAM(("Go back\n"));
                        // Go back a node, and check
                        this->input[i]->wire[j]->node->updateFeedback(nOut);
                    }
                }
            }

            return;
        }

        void queue(NativeAudioNode::NodeLink *in, NativeAudioNode::NodeLink *out) {

            // First connect block frames
            SPAM(("connect in node %p; out node %p\n", in->node, out->node));
            if (out->count == 0 && in->count == 0 && in->node != out->node) {
                this->frames[out->channel] = in->node->frames[in->channel];
            } else if (out->count == 1 || in->count == 1 || in->node == out->node) {
                // Multiple input or output on same channel
                // need to use an internal buffer
                this->frames[out->channel] = (float *) calloc(256, 4);
                SPAM(("Using custom frames\n"));
            } 

            // Then connect wires
            NodeLink *tmp = this->input[out->channel];
            tmp->wire[tmp->count] = new NodeIO(in->node, in->node->frames[in->channel]);
            SPAM(("frame %p\n", in->node->frames[in->channel]));
            SPAM(("Assigning input on %p wire %d on channel %d to %p\n", this, tmp->count , out->channel, in->node));

            // And update input node wires (this is informative)
            tmp = in->node->output[in->channel];
            tmp->wire[tmp->count] = new NodeIO(out->node, out->node->frames[out->channel]);

            in->count++;
            out->count++;

            // Check if wire created a feedback somewhere
            this->updateFeedback(out->node);
        }

        bool recurseGetData();

        virtual bool process() = 0;

        ~NativeAudioNode() {
            for (int i = 0; i < NATIVE_AUDIONODE_ARGS_SIZE; i++) {
                if (this->args[i] != NULL) {
                    delete this->args[i];
                }
            }
        }
    private:
        void post(ArgType type, void *ptr) {
        }
        NativeAudioParameters *params;
        NativeSharedMessages *sharedMsg;
};

class NativeAudioNodeTarget : public NativeAudioNode
{
    public :
        NativeAudioNodeTarget(int inCount, int outCount, NativeAudioParameters *params) 
            : NativeAudioNode(inCount, outCount, params)
        { 
            printf("Target init\n");

        }

        virtual bool process()
        {
            SPAM(("process called on target\n"));
            return true;
        }
};

class NativeAudioNodeWirdo : public NativeAudioNode
{
    public :
        NativeAudioNodeWirdo (int inCount, int outCount, NativeAudioParameters *params) : NativeAudioNode(inCount, outCount, params) 
        {
            printf("Wirdo init\n");
            printf("count %d/%d\n", inCount, outCount);
            /*
            for (int i = 0; i < inCount; i++) {
                this->inQueue[j]
            }
            */
        }

        float gain;

        virtual bool process() 
        {
            SPAM(("|process called on wirdo\n"));
            for (int i = 0; i < 256; i++) {
                this->frames[2][i] = this->frames[0][i];
                this->frames[3][i] = this->frames[1][i];
            }
            return true;
        }
};

class NativeAudioNodeGain : public NativeAudioNode
{
    public :
        NativeAudioNodeGain(int inCount, int outCount, NativeAudioParameters *params) 
            : NativeAudioNode(inCount, outCount, params) 
        {
            printf("gain init\n");
            this->args[0] = new ExportsArgs("gain", DOUBLE, &this->gain);
            this->args[1] = new ExportsArgs("gain", INT, &this->gain);
        }

        double gain;

        virtual bool process() 
        {
            SPAM(("|process called on gain\n"));
            for (int i = 0; i < 256; i++) {
                //SPAM(("gain data "));
                for (int j = 0; j < this->inCount; j++) {
                    this->frames[this->input[j]->channel][i] *= this->gain;
                 /*   SPAM(("%f", this->frames[this->input[j]->channel][i]));
                    if (j % 2 == 0) {
                        SPAM(("/"));
                    }*/
                }
                //SPAM(("\n"));
                /*
                this->frames[0][i] *= 0.4;
                this->frames[1][i] *= 0.5;
                */
                //printf("gain data %f/%f\n", this->frames[0][i], this->frames[1][i]);
            }
            return true;
        }
};

class NativeAudioNodeMixer : public NativeAudioNode
{
    public :
        NativeAudioNodeMixer(int inCount, int outCount, NativeAudioParameters *params) : NativeAudioNode(inCount, outCount, params) 
        {
            printf("Mixer init\n");
            printf("count %d/%d\n", inCount, outCount);
        }

        virtual bool process() 
        {
            SPAM(("|process called on mixer\n"));

            if (this->outCount == 2) {
                for (int i = 0; i < 256; i++) {
                    float tmpL, tmpR;

                    tmpL = tmpR = 0;

                    for (int j = 0; j < this->inCount; j+=2) {
                        tmpL += this->frames[j][i];
                        tmpR += this->frames[j+1][i];
                    }
                    this->frames[0][i] = tmpL/(this->inCount/2);
                    this->frames[1][i] = tmpR/(this->inCount/2);
                }
            } else if (this->outCount == 1) {
                float tmpL, tmpR;

                tmpL = tmpR = 0;
                for (int i = 0; i < 256; i++) {
                    for (int j = 0; j < this->inCount; j++) {
                        tmpL += this->frames[j][i];
                    }
                    this->frames[0][i] = tmpL/(this->inCount);
                }
            }
            return true;
        }
};
#endif
