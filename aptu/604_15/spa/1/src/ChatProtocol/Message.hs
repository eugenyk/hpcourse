{-# LANGUAGE BangPatterns, DeriveDataTypeable, FlexibleInstances, MultiParamTypeClasses, TemplateHaskell #-}
{-# OPTIONS_GHC -fno-warn-unused-imports #-}
module ChatProtocol.Message (Message(..)) where
import Prelude ((+), (/))
import qualified Prelude as Prelude'
import qualified Data.Typeable as Prelude'
import qualified Data.Data as Prelude'
import qualified Text.ProtocolBuffers.Header as P'
import qualified ChatProtocol.Message.Type as ChatProtocol.Message (Type)
 
data Message = Message{type' :: !(ChatProtocol.Message.Type), text :: !(P'.Seq P'.Utf8), author :: !(P'.Maybe P'.Utf8)}
             deriving (Prelude'.Show, Prelude'.Eq, Prelude'.Ord, Prelude'.Typeable, Prelude'.Data)
 
instance P'.Mergeable Message where
  mergeAppend (Message x'1 x'2 x'3) (Message y'1 y'2 y'3)
   = Message (P'.mergeAppend x'1 y'1) (P'.mergeAppend x'2 y'2) (P'.mergeAppend x'3 y'3)
 
instance P'.Default Message where
  defaultValue = Message P'.defaultValue P'.defaultValue P'.defaultValue
 
instance P'.Wire Message where
  wireSize ft' self'@(Message x'1 x'2 x'3)
   = case ft' of
       10 -> calc'Size
       11 -> P'.prependMessageSize calc'Size
       _ -> P'.wireSizeErr ft' self'
    where
        calc'Size = (P'.wireSizeReq 1 14 x'1 + P'.wireSizeRep 1 9 x'2 + P'.wireSizeOpt 1 9 x'3)
  wirePut ft' self'@(Message x'1 x'2 x'3)
   = case ft' of
       10 -> put'Fields
       11 -> do
               P'.putSize (P'.wireSize 10 self')
               put'Fields
       _ -> P'.wirePutErr ft' self'
    where
        put'Fields
         = do
             P'.wirePutReq 16 14 x'1
             P'.wirePutRep 26 9 x'2
             P'.wirePutOpt 34 9 x'3
  wireGet ft'
   = case ft' of
       10 -> P'.getBareMessageWith update'Self
       11 -> P'.getMessageWith update'Self
       _ -> P'.wireGetErr ft'
    where
        update'Self wire'Tag old'Self
         = case wire'Tag of
             16 -> Prelude'.fmap (\ !new'Field -> old'Self{type' = new'Field}) (P'.wireGet 14)
             26 -> Prelude'.fmap (\ !new'Field -> old'Self{text = P'.append (text old'Self) new'Field}) (P'.wireGet 9)
             34 -> Prelude'.fmap (\ !new'Field -> old'Self{author = Prelude'.Just new'Field}) (P'.wireGet 9)
             _ -> let (field'Number, wire'Type) = P'.splitWireTag wire'Tag in P'.unknown field'Number wire'Type old'Self
 
instance P'.MessageAPI msg' (msg' -> Message) Message where
  getVal m' f' = f' m'
 
instance P'.GPB Message
 
instance P'.ReflectDescriptor Message where
  getMessageInfo _ = P'.GetMessageInfo (P'.fromDistinctAscList [16]) (P'.fromDistinctAscList [16, 26, 34])
  reflectDescriptorInfo _
   = Prelude'.read
      "DescriptorInfo {descName = ProtoName {protobufName = FIName \".ChatProtocol.Message\", haskellPrefix = [], parentModule = [MName \"ChatProtocol\"], baseName = MName \"Message\"}, descFilePath = [\"ChatProtocol\",\"Message.hs\"], isGroup = False, fields = fromList [FieldInfo {fieldName = ProtoFName {protobufName' = FIName \".ChatProtocol.Message.type\", haskellPrefix' = [], parentModule' = [MName \"ChatProtocol\",MName \"Message\"], baseName' = FName \"type'\", baseNamePrefix' = \"\"}, fieldNumber = FieldId {getFieldId = 2}, wireTag = WireTag {getWireTag = 16}, packedTag = Nothing, wireTagLength = 1, isPacked = False, isRequired = True, canRepeat = False, mightPack = False, typeCode = FieldType {getFieldType = 14}, typeName = Just (ProtoName {protobufName = FIName \".ChatProtocol.Message.Type\", haskellPrefix = [], parentModule = [MName \"ChatProtocol\",MName \"Message\"], baseName = MName \"Type\"}), hsRawDefault = Nothing, hsDefault = Nothing},FieldInfo {fieldName = ProtoFName {protobufName' = FIName \".ChatProtocol.Message.text\", haskellPrefix' = [], parentModule' = [MName \"ChatProtocol\",MName \"Message\"], baseName' = FName \"text\", baseNamePrefix' = \"\"}, fieldNumber = FieldId {getFieldId = 3}, wireTag = WireTag {getWireTag = 26}, packedTag = Nothing, wireTagLength = 1, isPacked = False, isRequired = False, canRepeat = True, mightPack = False, typeCode = FieldType {getFieldType = 9}, typeName = Nothing, hsRawDefault = Nothing, hsDefault = Nothing},FieldInfo {fieldName = ProtoFName {protobufName' = FIName \".ChatProtocol.Message.author\", haskellPrefix' = [], parentModule' = [MName \"ChatProtocol\",MName \"Message\"], baseName' = FName \"author\", baseNamePrefix' = \"\"}, fieldNumber = FieldId {getFieldId = 4}, wireTag = WireTag {getWireTag = 34}, packedTag = Nothing, wireTagLength = 1, isPacked = False, isRequired = False, canRepeat = False, mightPack = False, typeCode = FieldType {getFieldType = 9}, typeName = Nothing, hsRawDefault = Nothing, hsDefault = Nothing}], keys = fromList [], extRanges = [], knownKeys = fromList [], storeUnknown = False, lazyFields = False, makeLenses = False}"
 
instance P'.TextType Message where
  tellT = P'.tellSubMessage
  getT = P'.getSubMessage
 
instance P'.TextMsg Message where
  textPut msg
   = do
       P'.tellT "type" (type' msg)
       P'.tellT "text" (text msg)
       P'.tellT "author" (author msg)
  textGet
   = do
       mods <- P'.sepEndBy (P'.choice [parse'type', parse'text, parse'author]) P'.spaces
       Prelude'.return (Prelude'.foldl (\ v f -> f v) P'.defaultValue mods)
    where
        parse'type'
         = P'.try
            (do
               v <- P'.getT "type"
               Prelude'.return (\ o -> o{type' = v}))
        parse'text
         = P'.try
            (do
               v <- P'.getT "text"
               Prelude'.return (\ o -> o{text = P'.append (text o) v}))
        parse'author
         = P'.try
            (do
               v <- P'.getT "author"
               Prelude'.return (\ o -> o{author = v}))