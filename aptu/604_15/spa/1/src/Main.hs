{-# LANGUAGE OverloadedStrings #-}
{-# LANGUAGE RecordWildCards   #-}

module Main where

import Control.Concurrent
import Control.Concurrent.Timer
import Control.Concurrent.Suspend
import Control.Monad
import Control.Concurrent.Async
import Network.Socket
import Data.Maybe
import Data.List.Split
import Data.Sequence

import System.Environment
import System.IO hiding (utf8)
import Text.Read
import Text.Printf
import GHC.Exts (toList)
import System.Process

import qualified Data.ByteString.Lazy as B

import Text.ProtocolBuffers.WireMessage
import Text.ProtocolBuffers.Basic (Utf8, uToString, uFromString)

import ChatProtocol.Message
import ChatProtocol.Message.Type


data Packet = BroadcastPacket { msg :: Message, source :: Handle }
            | TargetedPacket  { msg :: Message, target :: Handle }


type Bus = Chan Packet


type CommandQueue = Chan (Message, Handle)


getMessageText :: Message -> [String]
getMessageText msg = fmap uToString (toList $ text msg)


createMessageWithText :: String -> [String] -> Message
createMessageWithText author text = Message MESSAGE textUtf8Seq (Just authorUtf8)
    where
        textUtf8Seq :: Seq Utf8
        textUtf8Seq = fromList $ fmap uFromString text
        authorUtf8 :: Utf8
        authorUtf8 = uFromString author


-- Try to replace on standart
parseMessage :: B.ByteString -> Either String (Message, B.ByteString)
parseMessage content =
    case runGetOnLazy getVarInt content of
        (Left sizeParseError)       -> Left sizeParseError
        (Right (size, contentTail)) ->
            let (messageBuffer, contentTail') = B.splitAt size contentTail in
            case messageGet messageBuffer of
                (Left messageParseError) -> Left messageParseError
                (Right (msg, _))         -> Right (msg, contentTail')


-- parseMessage :: B.ByteString -> Either String (Message, B.ByteString)
-- parseMessage = messageWithLengthGet


dataReceiver :: Handle -> Bus -> CommandQueue -> IO ()
dataReceiver h bus commandQueue = do
    content <- B.hGetContents h
    parseMessageLoop content
        where
            parseMessageLoop c = do
                case parseMessage c of
                    (Left errMsg)     -> putStrLn errMsg
                    (Right (msg, c')) -> do
                        case type' msg of MESSAGE -> toBus msg
                                          COMMAND -> toCommandQueue msg
                        parseMessageLoop c'
            toBus msg = writeChan bus $ BroadcastPacket msg h
            toCommandQueue msg = writeChan commandQueue (msg, h)


dataSender :: Handle -> Bus -> IO ()
dataSender h source = forever $ do
    pack <- readChan source
    case pack of BroadcastPacket{..} -> when (source /= h) $ send msg
                 TargetedPacket{..}  -> when (target == h) $ send msg
        where
            send = (B.hPut h) . messageWithLengthPut


executeCommand :: String -> IO [String]
executeCommand cmd = fmap splitByLines $ cmdOut
    where
        cmdOut = readCreateProcess (shell cmd) ""
        splitByLines = splitOn "\n"


commandExecutor :: Bus -> CommandQueue -> IO ()
commandExecutor bus commandQueue = forever $ do
    (msg, target) <- readChan commandQueue
    let cmd = head $ getMessageText msg
    cmdOut <- executeCommand cmd
    writeChan bus $ TargetedPacket (createMessageWithText "SERVER" cmdOut) target


runServiceOnSocket :: Socket -> IO ()
runServiceOnSocket socket = do
    bus <- newChan
    commandQueue <- newChan
    forkIO $ commandExecutor bus commandQueue
    forever $ acceptOneClient bus commandQueue
        where
            acceptOneClient bus commandQueue = do
                (conn, _) <- accept socket
                h <- socketToHandle conn ReadWriteMode
                hSetBuffering h $ BlockBuffering (Just $ 1024 * 64)
                repeatedTimer (hFlush h) $ msDelay 300
                source <- dupChan bus
                let rcvr = dataReceiver h bus commandQueue
                let sndr = dataSender h source
                _ <- forkFinally (race sndr rcvr) (\_ -> do
                                                        putStrLn "closed"
                                                        hClose h)
                return ()



configureCapabilities :: IO ()
configureCapabilities = do
    args <- getArgs
    let capabilities = fromMaybe 4 (listToMaybe args >>= readMaybe) 
    putStrLn $ printf "Set num capabilities to: %d" capabilities
    setNumCapabilities capabilities


serverPort :: PortNumber
serverPort = 20053


main :: IO ()
main = withSocketsDo $ do
    configureCapabilities
    sock <- socket AF_INET Stream defaultProtocol
    setSocketOption sock ReuseAddr 1
    setSocketOption sock NoDelay 1
    bindSocket sock (SockAddrInet serverPort iNADDR_ANY)
    listen sock 100
    runServiceOnSocket sock

