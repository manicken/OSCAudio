/* OSC wrapper for AudioEffectFPB (float patch bytecode, see the manicken Design Tool++ repo, fpb/).
 *
 * Included by OSCAudioBase.h when the FPB library is available (__has_include(<AudioEffectFPB.h>)),
 * and registered for /dynamic/crOb as "AudioEffectFPB" via OSC_AUDIO_CLASSES_EXTRA.
 *
 * Methods (<root>/audio/<path>/...):
 *   /begin               start a streaming load
 *   /feed      b         the next piece of the container (any size; the tool sends 4 KB pieces)
 *   /end                 finish; the patch replaces the old one at the next audio block. Reply: int 0 or an FPB error
 *   /load      b         a whole container in one message. Reply as /end
 *   /loadFile  s         a container from the SD card (i.e. saved with /fs/save). Reply as /end
 *   /clear               remove the patch (silence)
 *   /setParam  if        parameter index, value. Reply: bool
 *   /getParam  i         Reply: float
 *   /numParams           Reply: int
 *   /lastError           Reply: int, the last load error (0 = OK)
 *   /cpu, /cpuMax        Reply: float, percent of one audio block the patch takes (last, highest)
 *   /isOverloaded        Reply: bool, the patch was stopped by the CPU guard (see AudioEffectFPB::setCpuLimit)
 *   /setCpuLimit f       percent of one block (default 70, 0 = no guard)
 */
#if !defined(_OSCAUDIOFPB_H_)
#define _OSCAUDIOFPB_H_

#include <AudioEffectFPB.h>

#include <SD.h>

class OSCAudioEffectFPB : public AudioEffectFPB, public OSCAudioBase
{
    public:
        OSCAudioEffectFPB(const char* _name) :  OSCAudioBase(_name, (AudioStream*) this) {}
#if defined(DYNAMIC_AUDIO_AVAILABLE)
        OSCAudioEffectFPB(const char* _name, OSCAudioGroup& grp) :  OSCAudioBase(_name, grp, (AudioStream*) this) {}
#endif

        void route(OSCMessage& msg, int addrOff, OSCBundle& reply)
        {
          int nameOff;
          if ((nameOff = isMine(msg,addrOff)) > 0)
          {
            addrOff += nameOff;
            char* nameOfTarget;
            nameOfTarget = (char*) alloca(getPathNameLength(this)+1);
            if (NULL != nameOfTarget)
              getPathNameTo(this,nameOfTarget);
            if (isTarget(msg,addrOff,"/begin",NULL)) {begin(); addReplyExecuted(msg,addrOff,reply,nameOfTarget);}
            else if (isTarget(msg,addrOff,"/feed","b")) {addReplyResult(msg,addrOff,reply,(int32_t) feedBlob(msg,0),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/end",NULL)) {addReplyResult(msg,addrOff,reply,(int32_t) end(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/loadFile","s")) {addReplyResult(msg,addrOff,reply,(int32_t) loadFile(msg),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/load","b")) {begin(); int e = feedBlob(msg,0); if (!e) e = end(); addReplyResult(msg,addrOff,reply,(int32_t) e,nameOfTarget);}
            else if (isTarget(msg,addrOff,"/clear",NULL)) {clear(); addReplyExecuted(msg,addrOff,reply,nameOfTarget);}
            else if (isTarget(msg,addrOff,"/setParam","if")) {addReplyResult(msg,addrOff,reply,setParam((uint16_t) msg.getInt(0),msg.getFloat(1)),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/getParam","i")) {addReplyResult(msg,addrOff,reply,getParam((uint16_t) msg.getInt(0)),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/numParams",NULL)) {addReplyResult(msg,addrOff,reply,(int32_t) numParams(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/lastError",NULL)) {addReplyResult(msg,addrOff,reply,(int32_t) lastError(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/cpu",NULL)) {addReplyResult(msg,addrOff,reply,cpuUsage(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/cpuMax",NULL)) {addReplyResult(msg,addrOff,reply,cpuUsageMax(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/isOverloaded",NULL)) {addReplyResult(msg,addrOff,reply,isOverloaded(),nameOfTarget);}
            else if (isTarget(msg,addrOff,"/setCpuLimit","f")) {setCpuLimit(msg.getFloat(0)); addReplyExecuted(msg,addrOff,reply,nameOfTarget);}
            else addReplyResult(msg,addrOff,reply,false,nameOfTarget,INVALID_METHOD);
          }
        }

    private:
        // feeds a blob argument in small pieces (no buffer as big as the blob is needed)
        int feedBlob(OSCMessage& msg, int position)
        {
          uint8_t buf[256];
          int len = (int) msg.getBlobLength(position);
          if (len < 0) return FPB_E_STATE;
          for (int off = 0; off < len; off += sizeof(buf))
          {
            int n = len - off < (int) sizeof(buf) ? len - off : (int) sizeof(buf);
            if (msg.getBlob(position, buf, sizeof(buf), off, n) != n) return FPB_E_STATE;
            int e = feed(buf, n);
            if (e) return e;
          }
          return FPB_OK;
        }

        int loadFile(OSCMessage& msg)
        {
          char fn[64];
          msg.getString(0, fn, sizeof(fn));
          File f = SD.open(fn);
          if (!f) return FPB_E_INCOMPLETE;
          begin();
          uint8_t buf[512];
          int e = FPB_OK;
          while (!e && f.available())
          {
            int n = f.read(buf, sizeof(buf));
            if (n <= 0) break;
            e = feed(buf, n);
          }
          f.close();
          return e ? e : end();
        }
};

#if defined(DYNAMIC_AUDIO_AVAILABLE)
#define OSC_AUDIO_CLASSES_EXTRA OSC_CLASS(AudioEffectFPB,OSCAudioEffectFPB,noRequirementCheck)
#else
#define OSC_AUDIO_CLASSES_EXTRA OSC_CLASS(AudioEffectFPB)
#endif

#endif // !defined(_OSCAUDIOFPB_H_)
