(use-modules (srfi srfi-1) (srfi srfi-13) (srfi srfi-9))

(define-record-type <statemachine>
	(make-statemachine state transitions hooks)
	statemachine?
	(state statemachine-state statemachine-state!)
	(transitions statemachine-transitions)
	(hooks statemachine-hooks)
)

(define (add-path machine first-key second-key value)
	(if (equal? (hash-ref machine first-key) #f)
		(hash-set! machine first-key (make-hash-table)) 
	)
	(let* ((transition-to-state-map (hash-ref machine first-key)) (second-key-value (hash-ref transition-to-state-map second-key)))
		(if (and (not (equal? second-key-value #f)) (not(equal? second-key-value value)))
			(throw 'duplicate-transition-different-to-state-error)
		)
		(hash-set! transition-to-state-map second-key value)
	)
)
(define (get-state-at-transition machine from-state transition)
	(if (equal? (hash-ref (statemachine-transitions machine) from-state) #f)
		(throw 'from-state-does-not-exist)
	)
	(hash-ref (hash-ref (statemachine-transitions machine) from-state) transition)
)
(define (add-transition-callback machine transition callback)
	(hash-set! (statemachine-transitions machine) transition callback)
)
(define (create-state machine from-state to-state transition)	
	(add-path (statemachine-transitions machine) from-state transition to-state)
)
(define (move-transition machine transition)
	(statemachine-state! machine (get-state-at-transition machine (statemachine-state machine) transition))
	(let ((callback (hash-ref (statemachine-transitions machine) transition)))
		(if (not (equal? callback #f))
			(callback)
		)
	)
)
(define (new-state-machine initial-state)
	(make-statemachine initial-state (make-hash-table) (make-hash-table))
)

(define-syntax ::
	(syntax-rules(: - > query | goto) 
		((:: machine | transition - callback) (add-transition-callback machine transition callback))
		((:: machine from-state -> to-state) (create-state machine from-state to-state 'default))
		((:: machine from-state -> to-state | transition) (create-state machine from-state to-state transition))
		((:: machine query from-state | transition) (get-state-at-transition machine from-state transition))
		((:: machine query from-state) (:: machine query from-state | 'default))
		((:: machine goto transition) (move-transition machine transition))
	)
)
