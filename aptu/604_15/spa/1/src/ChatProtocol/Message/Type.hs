{-# LANGUAGE BangPatterns, DeriveDataTypeable, FlexibleInstances, MultiParamTypeClasses, TemplateHaskell #-}
{-# OPTIONS_GHC -fno-warn-unused-imports #-}
module ChatProtocol.Message.Type (Type(..)) where
import Prelude ((+), (/), (.))
import qualified Prelude as Prelude'
import qualified Data.Typeable as Prelude'
import qualified Data.Data as Prelude'
import qualified Text.ProtocolBuffers.Header as P'
 
data Type = COMMAND
          | MESSAGE
          deriving (Prelude'.Read, Prelude'.Show, Prelude'.Eq, Prelude'.Ord, Prelude'.Typeable, Prelude'.Data)
 
instance P'.Mergeable Type
 
instance Prelude'.Bounded Type where
  minBound = COMMAND
  maxBound = MESSAGE
 
instance P'.Default Type where
  defaultValue = COMMAND
 
toMaybe'Enum :: Prelude'.Int -> P'.Maybe Type
toMaybe'Enum 1 = Prelude'.Just COMMAND
toMaybe'Enum 2 = Prelude'.Just MESSAGE
toMaybe'Enum _ = Prelude'.Nothing
 
instance Prelude'.Enum Type where
  fromEnum COMMAND = 1
  fromEnum MESSAGE = 2
  toEnum = P'.fromMaybe (Prelude'.error "hprotoc generated code: toEnum failure for type ChatProtocol.Message.Type") . toMaybe'Enum
  succ COMMAND = MESSAGE
  succ _ = Prelude'.error "hprotoc generated code: succ failure for type ChatProtocol.Message.Type"
  pred MESSAGE = COMMAND
  pred _ = Prelude'.error "hprotoc generated code: pred failure for type ChatProtocol.Message.Type"
 
instance P'.Wire Type where
  wireSize ft' enum = P'.wireSize ft' (Prelude'.fromEnum enum)
  wirePut ft' enum = P'.wirePut ft' (Prelude'.fromEnum enum)
  wireGet 14 = P'.wireGetEnum toMaybe'Enum
  wireGet ft' = P'.wireGetErr ft'
  wireGetPacked 14 = P'.wireGetPackedEnum toMaybe'Enum
  wireGetPacked ft' = P'.wireGetErr ft'
 
instance P'.GPB Type
 
instance P'.MessageAPI msg' (msg' -> Type) Type where
  getVal m' f' = f' m'
 
instance P'.ReflectEnum Type where
  reflectEnum = [(1, "COMMAND", COMMAND), (2, "MESSAGE", MESSAGE)]
  reflectEnumInfo _
   = P'.EnumInfo (P'.makePNF (P'.pack ".ChatProtocol.Message.Type") [] ["ChatProtocol", "Message"] "Type")
      ["ChatProtocol", "Message", "Type.hs"]
      [(1, "COMMAND"), (2, "MESSAGE")]
 
instance P'.TextType Type where
  tellT = P'.tellShow
  getT = P'.getRead