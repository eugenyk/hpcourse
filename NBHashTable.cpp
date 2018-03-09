#include "NBHashTable.h"

VersionState::VersionState(int version, VersionState::State state) {
	set(version, state);
}

void VersionState::set(int version, VersionState::State state) {
	this->store(makeRaw(version, state));
}

VersionState::State VersionState::getState(int raw) {
	int sum = 0;
	
	
	for(int i = NUM_STATE_BITS-1; i>=0; i--)
		sum += (1 << i) * (int)(getBit(raw, i));
	
	return (VersionState::State) sum;
}

int VersionState::getVersion(int raw) {
	return raw >> NUM_STATE_BITS;
}
	

int VersionState::makeRaw(int version, VersionState::State state) {
	version = version << NUM_STATE_BITS;
	
	
        //Младшие биты, отражающие состояние
	for(int i = NUM_STATE_BITS-1; i >= 0; i--) 
		version = setBit(version, getBit(state, i), i);
	
	return version;
}

bool VersionState::getBit(int val, int bit) {
	return val & (0x1 << bit);
}

int VersionState::setBit(int num, bool value, int position) {
	if(value) return num | (0x1 << position);
	else return num & ~(0x1 << position);
}

const char *VersionState::getStateString(VersionState::State state) {	
	switch(state) {
		case VersionState::State::BUSY:
			return "busy";
		case VersionState::State::MEMBER:
			return "member";
		case VersionState::State::INSERTING:
			return "inserting";
		case VersionState::State::EMPTY:
			return "empty";
		case VersionState::State::COLLIDED:
			return "collided";
		case VersionState::State::VISIBLE:
			return "visible";
		default:
			return "unknown";
	}
}

ProbeBound::ProbeBound(int pb, bool scanning) {
	set(pb, scanning);
}

void ProbeBound::set(int pb, bool scanning) {
	this->store(makeRaw(pb, scanning));
}


// Применение:
// Использовать ProbeBound-> load для получения необработанного значения.
// Когда у нас есть исходное значение, мы можем получить фактические значения, используя эти статические функции.
// Ex:
// ProbeBound p (10);
// int rawB = p-> load ();
// printf («Оценка слота равна% d \ n», ProbeBound :: getBound (raw));

int ProbeBound::getBound(int raw) {
	return raw >> 1;
}

bool ProbeBound::isScanning(int raw) {
	int rawBit = raw & 0x1;
	return rawBit == 0x1;
}


//Приватные фкнкции ниже
int ProbeBound::makeRaw(int pb, bool scanning) {
	pb = (pb >= 0) ? pb << 1 : 0;
	return setBit(pb, scanning, 0);
}

void ProbeBound::printBits(int ref) {
	
        // Count с двумя указателями, один для MSB для LSB
        // Когда MSB подсчитан ниже нуля, мы закончили.
	for(int l=sizeof(int)*8-1; l >= 0; l--) {
		printf("%d", getBit(ref, l) ? 1 : 0);
	}
	
	printf("\n");
}

bool ProbeBound::getBit(int val, int bit) {
	return val & (0x1 << bit);
}


int ProbeBound::setBit(int num, bool value, int position) {
	if(value) return num | (0x1 << position);
	else return num & ~(0x1 << position);
}


typedef VersionState::State VSTATE;

NBHashTable::NBHashTable(int ks) {
	int i;
	
	kSize = ks;
	buckets = new BucketT[kSize];
	bounds = new ProbeBound[kSize];
	
	for(i = 0; i < kSize; i++) {
		buckets[i].vs = new VersionState(0,VersionState::State::EMPTY);
		buckets[i].key = EMPTY_FLAG;
	}
}

NBHashTable::~NBHashTable() {
	delete bounds;
	delete buckets;
}

bool NBHashTable::insert(NBType k) {
	int hashIndex = hash(k);
	int i = -1;

	int version;
	VersionState::State state;

	while (true) {
		

        //Проверки, чтобы убедиться, что Таблица не полная

        if (++i >= kSize) {
			return false;
		}
        
 
        //Получает текущее состояние версии слота и разбивает его на две части
  
		int vs = getBucketValue(hashIndex, i)->vs->load();
		version = VersionState::getVersion(vs);
		state = VersionState::getState(vs);


        //Создает два объекта для сравнения 
  
		int cmpVs = VersionState::makeRaw(version,VersionState::State::EMPTY);
		int newVs = VersionState::makeRaw(version,VersionState::State::BUSY);
        
    
        //Выполняет сравнение и обмен для изменения состояния (пуст -> занят)
		if (getBucketValue(hashIndex, i)->vs->compare_exchange_strong(cmpVs, newVs, std::memory_order_release, std::memory_order_relaxed)) {
			break;
		}
	}
    
 
    // Обновление значения ключа слота
	getBucketValue(hashIndex, i)->key = k;
	
    while (true) {
	
                // установка состояния слота  как visible
        getBucketValue(hashIndex, i)->vs->set(version, VersionState::State::VISIBLE);
		

        //обновление probe bound слота
 
        conditionallyRaiseBound(hashIndex, i);
		
    
         // установка состояния слота  как INSERTING
        getBucketValue(hashIndex, i)->vs->set(version,VersionState::State::INSERTING);
		
   
        //осуществляет содействие в проведении операции
        bool r = assist(k,hashIndex,i,version);
		
      
        // Создает версию состояния для операции сравнения
        VersionState collidedVs(version,VersionState::State::COLLIDED);
        
       
        //Находится ли текущий слот в состоянии коллизии
		if (getBucketValue(hashIndex, i)->vs->load() != collidedVs.load()) {
			return true;
		}
        
        //Если вспомогательная операция вернулась с ошибкой, обновите bound, увеличьте версию и опустошите слот
		if (!r) {
  
            // Увеличить bound
			conditionallyLowerBound(hashIndex, i);
			
        
            //Создать новую версию состояния с обновленным и пустым состоянием
  
            VersionState emptyVs(version + 1,VersionState::State::EMPTY);
            
      
            //Обновить версию состояния ведра
			getBucketValue(hashIndex, i)->vs->store(emptyVs);
			return false;
		}
        
  
        // Увеличить версию
		version++;

	}


}

bool NBHashTable::put(NBType n) {
	return this->insert(n);
}

bool NBHashTable::contains(NBType k) {
	int hashIndex = hash(k);
	int max = getProbeBound(hashIndex);
	for(int jumps = 0; jumps <= max; jumps++) {
		int vs = getBucketValue(hashIndex, jumps)->vs->load();
		int version = VersionState::getVersion(vs);
		VersionState::State state = VersionState::getState(vs);

		if (state == VersionState::State::MEMBER && getBucketValue(hashIndex, jumps)->key == k) {
			VersionState newVs(version,VersionState::State::MEMBER);
			if (getBucketValue(hashIndex, jumps)->vs->load() == newVs.load()) {
				return true;
			}
		}
	}
	return false;
}

int NBHashTable::size() {
	return kSize;
}

bool NBHashTable::remove(NBType n) {
	int hashIndex = hash(n);
	int max = getProbeBound(hashIndex);
	for(int jumps = 0; jumps <= max; jumps++) {
		int vs = getBucketValue(hashIndex, jumps)->vs->load();
		int version = VersionState::getVersion(vs);
		VersionState::State state = VersionState::getState(vs);

		if (state == VersionState::State::MEMBER && getBucketValue(hashIndex, jumps)->key == n) {
			int cmpVs = VersionState::makeRaw(version,VersionState::State::MEMBER);
			int newVs = VersionState::makeRaw(version,VersionState::State::BUSY);

			if (getBucketValue(hashIndex, jumps)->vs->compare_exchange_strong(cmpVs, newVs, std::memory_order_release, std::memory_order_relaxed)) {
				conditionallyLowerBound(hashIndex, jumps);
				VersionState newVs(version + 1,VersionState::State::EMPTY);
				getBucketValue(hashIndex, jumps)->vs->store(newVs);
				return true;
			}
		}
	}
	return false;
}



// Хеш


int NBHashTable::hash(NBType n) {
	return (n % kSize);
}


void NBHashTable::printHashTableInfo() {
	mainMutex.lock();
	printf("Bounds:\t| ");
	for (int i = 0; i < kSize; i++) {
		printf("%-8d | ", ProbeBound::getBound(bounds[i].load()));
	}
	printf("\nKeys:\t| ");
	for (int i = 0; i < kSize; i++) {
		if(buckets[i].key == EMPTY_FLAG) printf("%-8s | ", "-");
		else printf("%-8d | ", buckets[i].key);
	}
	printf("\nScan:\t| ");
	for (int i = 0; i < kSize; i++) {
		printf("%-8s | ", ProbeBound::isScanning(bounds[i].load()) ? "true" : "false");
	}
	printf("\nState:\t| ");
	for (int i = 0; i < kSize; i++) {
		printf("%-8s | ", VersionState::getStateString(VersionState::getState(buckets[i].vs->load())));
	}
	printf("\n\n");
	mainMutex.unlock();
}


// Bucket (слот)

BucketT* NBHashTable::getBucketValue(int startIndex, int probeJumps) {
	return &buckets[(startIndex + (probeJumps * (probeJumps + 1)) /2) % kSize];
}

bool NBHashTable::doesBucketContainCollision(int startIndex, int probeJumps) {
	int vs = getBucketValue(startIndex, probeJumps)->vs->load();
	int version1 = VersionState::getVersion(vs);
	VersionState::State state1 = VersionState::getState(vs);
	if (state1 == VersionState::State::VISIBLE || state1 == VersionState::State::INSERTING || state1 == VersionState::State::MEMBER) {
		if (hash(getBucketValue(startIndex, probeJumps)->key) == startIndex) {
				int vs2 = getBucketValue(startIndex, probeJumps)->vs->load();
				int version2 = VersionState::getVersion(vs2);
				VersionState::State state2 = VersionState::getState(vs2);
				if (state2 == VersionState::State::VISIBLE || state2 == VersionState::State::INSERTING || state2 == VersionState::State::MEMBER) {
					if (version1 == version2) {
						return true;
					}
				}
		}
	}
	return false;
}


// Bounds

void NBHashTable::initProbeBound(int index) {
	bounds[index].set(0, false);
}

int NBHashTable::getProbeBound(int startIndex) {
	return ProbeBound::getBound(bounds[startIndex].load());
}

void NBHashTable::conditionallyRaiseBound(int startIndex, int probeJumps) {
	
	if(DEBUG) {
		mainMutex.lock();
		printf("Conditionally raising bound for bucket[%d], there were %d jumps\n", startIndex, probeJumps);
		mainMutex.unlock();
	}
	
	while (true) {
		int pb = bounds[startIndex].load();
		int oldBound = ProbeBound::getBound(pb);
		bool scanning = ProbeBound::isScanning(pb);

		int newBound = std::max(oldBound, probeJumps);
		ProbeBound newPb(newBound, false);
		
		if (bounds[startIndex].compare_exchange_strong(pb, newPb.load())) {
			return;
		}
	}
}

void NBHashTable::conditionallyLowerBound(int startIndex, int probeJumps) {
	int pb = bounds[startIndex].load();
	int bound = ProbeBound::getBound(pb);
	bool scanning = ProbeBound::isScanning(pb);
	if (scanning) {
		ProbeBound newPbObj(false,bound);
		int newPb = newPbObj.load();
		bounds[startIndex].compare_exchange_strong(pb, newPb, std::memory_order_release, std::memory_order_relaxed);
	}
	if (probeJumps > 0) {
		int cmpPb = ProbeBound::makeRaw(false,probeJumps);
		int newPb = ProbeBound::makeRaw(true,probeJumps);
		while (bounds[startIndex].compare_exchange_strong(cmpPb, newPb, std::memory_order_release, std::memory_order_relaxed)) {
			int i = probeJumps - 1;
			while (i > 0 && !doesBucketContainCollision(startIndex,probeJumps)) {
				i--;
			}

			int cmpPb2 = ProbeBound::makeRaw(true,probeJumps);
			int newPb2 = ProbeBound::makeRaw(false,i);
			bounds[startIndex].compare_exchange_strong(cmpPb2, newPb2, std::memory_order_release, std::memory_order_relaxed);
		}
	}
}


//Попробуем вставить ключ в индекс i
//ключ - это значение, которое мы пытаемся вставить
//h - это хэш-ключ или начальный индекс
//i - число отсчетов от h, так что i = 0 при первом запуске
//ver_i - это версия в слоте [h, i]
bool NBHashTable::assist(NBType key, int h, int i, int ver_i) {
	int max = getProbeBound(h);
	
	
        //Просмотр все возможных количеств прыжков

	for(int j = 0; j < max; j++) {
		
		
                //если мы смотрим.видим на тот же индекс, пропускаем его
		if(i==j) continue;
			
		
                // Получение атомарно версии/состояни 
                

		int rawVS = getBucketValue(h, j)->vs->load();
		int ver_j = VersionState::getVersion(rawVS);
		VSTATE state_j = VersionState::getState(rawVS);
		
		
                // Если это состояние вставляется, и оно имеет тот же ключ, который мы пытаемся вставить
		if(state_j == VSTATE::INSERTING && getBucketValue(h, j)->key == key) {
			
			
                        // Если он был обнаружен ранее в probe sequence
			if(j < i) {
				
				VersionState vsj_ins(ver_j, VSTATE::INSERTING);
				if (getBucketValue(h, j)->vs->load() == vsj_ins.load()) {
					int vsi_ins = VersionState::makeRaw(ver_i, VSTATE::INSERTING);
					getBucketValue(h, i)->vs->compare_exchange_strong(vsi_ins, VersionState::makeRaw(ver_i, VSTATE::COLLIDED), std::memory_order_release, std::memory_order_relaxed);
					return assist(key,h,j,ver_j);
				}
				
			} else {
				
				VersionState vsi_ins(ver_i, VSTATE::INSERTING);
				if(getBucketValue(h, i)->vs->load() == vsi_ins.load()) {
					int vsj_ins = VersionState::makeRaw(ver_j, VSTATE::INSERTING);
					getBucketValue(h,j)->vs->compare_exchange_strong(vsj_ins, VersionState::makeRaw(ver_j, VSTATE::COLLIDED), std::memory_order_release, std::memory_order_relaxed);
				}
			}
		}
		
	
                // Захватываем нашу версию снова
		rawVS = getBucketValue(h,j)->vs->load();
		ver_j = VersionState::getVersion(rawVS);
		state_j = VersionState::getState(rawVS);
		
		
                //Если мы обнаружим, что наше значение уже является членом(записано), тогда выйдем немедленно
		if(state_j == VSTATE::MEMBER && getBucketValue(h, j)->key == key) {
			VersionState vsj_mem(ver_j, VSTATE::MEMBER);
			if(getBucketValue(h, j)->vs->load() == vsj_mem.load()) {
				int vsi_ins = VersionState::makeRaw(ver_i, VSTATE::INSERTING);
				getBucketValue(h, i)->vs->compare_exchange_strong(vsi_ins, VersionState::makeRaw(ver_i, VSTATE::COLLIDED), std::memory_order_release, std::memory_order_relaxed);
				return false;
			}
		}
	}
	

        //Если мы здесь, значит, мы наконец  добавили индекс , наконец, мы можем пометить его как член!
      
	int vsi_ins = VersionState::makeRaw(ver_i, VSTATE::INSERTING);
	getBucketValue(h,i)->vs->compare_exchange_strong(vsi_ins, VersionState::makeRaw(ver_i, VSTATE::MEMBER), std::memory_order_release, std::memory_order_relaxed);
	return true;
}
