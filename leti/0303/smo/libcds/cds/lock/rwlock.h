//$$CDS-header$$

#ifndef __CDS_RWLOCK_H
#define __CDS_RWLOCK_H

#include <cds/atomic.h>
#include <cds/backoff_strategy.h>

#include <cds/details/noncopyable.h>

/*
    Классы Reader-Writer locks, построенные на CAS.
    Основные идеи и алгоритмы взяты из работы:
        John M. Mellor-Crummey, Michael L. Scott "Scalable Reader-Writer Synchronization for Shared-Memory Multiprocessors", 1991

    Преимущества:
        - не используют сложные структуры данных (типа очередей), - только одно 32- или 64-битное целое.
        - не требуют поддержки со стороны операционной системы или ядра
    Недостатки:
        - используют активное ожидание (aka spin locks) с backoff в случае неудачи.
        - в случае краха thread активного или ожидающего читателя/писателя доступ к ресурсу может быть заблокирован

    Классы:
        SpinWPref        writer preference, no writer ordering RW-lock
                        RW-lock с преимуществом писателя, без упорядочения писателей по времени поступления запроса на запись
        SpinWPrefOrd    writer preference, writer ordering RW-lock
                        RW-lock с преимуществом писателя, с упорядочением писателей по времени поступления запроса
        SpinRPref       Reader preference, no writer ordering RW-lock
                        RW-lock с преимуществом читателей. Писатели не упорядочены по времени поступления запроса на запись
*/

///@cond
namespace cds {
    namespace lock {

        /* RWSpinWPrefT  *******************************************************************************
            RW-lock с преимуществом писателя, без упорядочения писателей по времени поступления запроса
        ***********************************************************************************************/
        template <class BACKOFF>
        class RWSpinWPrefT: cds::details::noncopyable
        {
            union Barrier {
                struct {
                    unsigned int readerCount    : 32 / 2;        // число активных читателей
                    unsigned int writerCount    : 32 / 2 - 1;    // число активных и ожидающих писателей
                    unsigned int writerActive   : 1 ;            // 1 - писатель активен
                };
                atomic32_t         nAtomic            ;
            } ;

            volatile Barrier     m_Barrier   ;   // управляющая структура

        public:
            RWSpinWPrefT() throw()
            {
                m_Barrier.nAtomic = 0 ;
            }
            ~RWSpinWPrefT() throw()
            {
                assert( m_Barrier.nAtomic == 0 ) ;
            }

            // Вход читателя
            // Читатель получает доступ только тогда, когда нет активнго/ожидающих писателей
            void rlock()
            {
                BACKOFF backoff    ;
                Barrier bnew, b ;
                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    bnew.readerCount++      ;
                    bnew.writerActive = b.writerActive = 0  ;
                    bnew.writerCount = b.writerCount = 0    ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))    // OK, если нет писателей
                        break   ;
                    backoff() ;
                }
            }

            // Выход читателя
            void runlock()
            {
                BACKOFF backoff    ;
                Barrier bnew, b ;
                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    bnew.readerCount--      ;
                    assert( b.writerActive == 0 ) ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))    // уменьшаем количество читателей
                        break   ;
                    backoff()    ;
                }
            }

            // Вход писателя
            // Писатель сначала блокирует доступ читателям, приходящим позже, а затем ожидает освобождения ресурса:
            //  окончания работы активных читателей и активгого писателя. Доступ получает один из ожидающих писателей.
            void wlock()
            {
                Barrier bnew, b ;
                BACKOFF backoff    ;

                // Блокируем вновь потупающих читателей
                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    bnew.writerCount++      ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }

                backoff.reset() ;

                // Ожидаем, когда закончат читатели и активный писатель, если есть
                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    bnew.readerCount =
                        b.readerCount = 0   ;
                    b.writerActive = 0      ;
                    bnew.writerActive = 1   ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            // Выход писателя
            void wunlock()
            {
                Barrier bnew, b ;
                BACKOFF backoff    ;

                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    assert( b.writerActive == 1 ) ;
                    assert( b.readerCount == 0 )  ;
                    bnew.writerActive = 0   ;
                    --bnew.writerCount      ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            bool isWriting() const
            {
                return m_Barrier.writerActive == 1 ;
            }

            bool isReading() const
            {
                return m_Barrier.readerCount != 0    ;
            }
        };
        typedef RWSpinWPrefT< backoff::LockDefault >    RWSpinWPref ;

        /* RWSpinWPrefOrdT  ******************************************************************************
        RW-lock с преимуществом писателя, с упорядочением писателей по времени поступления запроса
            При входе писатель блокирует доступ читателям, получает билет, управляющий очередностью доступа
            писателей, и ждет, когда наступит его очередь согласно полученного билета.
        ***********************************************************************************************/
        template <class BACKOFF>
        class RWSpinWPrefOrdT: cds::details::noncopyable
        {
            union Barrier {
                struct {
                    unsigned short int  workerCount ;     // число входящих пользователей (читателей/писателей)
                    unsigned short int  writerCount ;     // число ожидающих или активных писателей
                    unsigned short int  doneCount   ;     // число исходящих (закончивших) пользователей
                    unsigned short int  writerActive;     // =1 - писатель активен
                };
                atomic64_t  nAtomic   ;
            };
            volatile Barrier     m_Barrier   ;

        public:
            RWSpinWPrefOrdT() throw()
            {
                static_assert( sizeof( m_Barrier ) == sizeof( m_Barrier.nAtomic ), "Structure size error") ;
                m_Barrier.nAtomic = 0 ;
            }
            ~RWSpinWPrefOrdT() throw()
            {
                assert( m_Barrier.writerCount == 0 ) ;
            }

            // Вход читателя
            // Читатель получает доступ только тогда, когда нет активного/ожидающих писателей
            void rlock()
            {
                Barrier b, bnew ;
                BACKOFF backoff    ;

                while (true) {
                    b.nAtomic = m_Barrier.nAtomic    ;
                    b.writerCount = 0            ;
                    bnew.nAtomic = b.nAtomic    ;
                    ++bnew.workerCount            ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            // Выход читателя
            void runlock()
            {
                Barrier bnew, b ;
                BACKOFF backoff    ;

                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    ++bnew.doneCount        ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            // Вход писателя
            // Писатель сначала блокирует доступ читателям, приходящим позже, и получает билет на вход
            //  (билет определяет порядок писателей). Затем писатель ожидает своего билета.
            void wlock()
            {
                Barrier bnew, b ;
                unsigned int nTicket    ;
                BACKOFF backoff    ;

                // Блокируем вновь поступающих читателей и получаем билет (ticket) на вход
                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    ++bnew.writerCount              ;
                    nTicket = bnew.workerCount      ;
                    ++bnew.workerCount              ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }

                backoff.reset() ;

                // Ожидаем своей очереди (по выданному билету)
                while (true) {
                    b.nAtomic = m_Barrier.nAtomic    ;
                    b.doneCount = nTicket   ;
                    bnew.nAtomic = b.nAtomic;
                    b.writerActive = 0      ;
                    bnew.writerActive = 1   ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            // Выход писателя
            void wunlock()
            {
                Barrier bnew, b ;
                BACKOFF backoff    ;

                while (true) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    --bnew.writerCount      ;
                    ++bnew.doneCount        ;
                    bnew.writerActive = 0   ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            bool isWriting() const
            {
                return m_Barrier.writerActive != 0    ;
            }

            bool isReading() const
            {
                return m_Barrier.writerCount == 0 && m_Barrier.workerCount != 0    ;
            }
        } ;
        typedef RWSpinWPrefOrdT< backoff::LockDefault > RWSpinWPrefOrd ;


        /* SpinRPrefT  *********************************************************************************
            RW-lock с преимуществом читателей, без упорядочения писателей по времени поступления запроса
        ***********************************************************************************************/
        template <class BACKOFF>
        class RWSpinRPrefT: cds::details::noncopyable
        {
            typedef    atomic32u_t    TAtomic    ;
            union Barrier {
                struct {
                    TAtomic        readerCount:  32 - 1 ;  // число активных читателей
                    TAtomic        writerActive: 1 ;        // 1 - активен писатель
                };
                TAtomic        nAtomic ;
            };
            volatile Barrier     m_Barrier ;
        public:
            RWSpinRPrefT() throw()
            {
                m_Barrier.nAtomic = 0   ;
            }
            ~RWSpinRPrefT() throw()
            {
                assert( m_Barrier.nAtomic == 0 ) ;
            }

            void rlock()
            {
                Barrier b, bnew ;
                BACKOFF backoff    ;

                while ( true ) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    ++bnew.readerCount      ;
                    bnew.writerActive = b.writerActive = 0      ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ) )
                        break   ;
                    backoff()    ;
                }
            }

            void runlock()
            {
                Barrier b, bnew ;
                BACKOFF backoff    ;

                while ( true ) {
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    assert( b.writerActive == 0 ) ;
                    --bnew.readerCount      ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ) ) {
                        break   ;
                    }
                    backoff()    ;
                }
            }

            void wlock()
            {
                Barrier b ;
                BACKOFF backoff    ;

                b.writerActive = 1  ;
                b.readerCount = 0   ;
                while ( true ) {
                    if ( CAS( &m_Barrier.nAtomic, (TAtomic) 0, b.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            void wunlock()
            {
                BACKOFF backoff    ;

                while ( true ) {
                    Barrier b, bnew         ;
                    bnew.nAtomic = b.nAtomic = m_Barrier.nAtomic    ;
                    bnew.writerActive = 0   ;
                    if ( CAS( &m_Barrier.nAtomic, b.nAtomic, bnew.nAtomic ))
                        break   ;
                    backoff()    ;
                }
            }

            bool isWriting() const
            {
                return m_Barrier.writerActive == 1 ;
            }

            bool isReading() const
            {
                return m_Barrier.readerCount != 0    ;
            }
        };
        typedef RWSpinRPrefT<backoff::LockDefault>    RWSpinRPref   ;

        /* AutoR/W ***********************************************************************
            Автоматические объекты для readers/writers
        **********************************************************************************/
        template <class RWLOCK>
        class AutoR {
            RWLOCK& m_lock  ;
        public:
            AutoR( RWLOCK& lock )
                : m_lock( lock )
            {
                m_lock.rlock()  ;
            }
            ~AutoR()
            {
                m_lock.runlock() ;
            }
        };

        template <class RWLOCK>
        class AutoW {
            RWLOCK& m_lock  ;
        public:
            AutoW( RWLOCK& lock )
                : m_lock( lock )
            {
                m_lock.wlock() ;
            }
            ~AutoW()
            {
                m_lock.wunlock() ;
            }
        };
    }   // namespace lock

    // Синонимы самых распространенных классов
    //

    typedef lock::RWSpinWPref        RWSpinLock    ;

} // namespace cds

///@endcond


#endif // #ifndef __CDS_RWLOCK_H
