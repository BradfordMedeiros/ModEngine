
(define players '())
(define active-index -1)

(define (print-players)
  (display "players: \n")
  (display players)
  (display "\n")
)

(define (create-player name)
  (let ((objectid (mk-obj name "./res/models/box/box.obj" (list (* 10 (length players)) 0 0))))
    (set! players (cons (cons name objectid) players))
  )
  (display (string-append "player created: " name))
)
(define (delete-player name)
  (set! players (assoc-remove! players name))
  (display (string-append "player deleted: " name))
)

(define (on-player-join connection-hash)
  (create-player connection-hash)
)
(define (on-player-leave connection-hash)
  (delete-player connection-hash)
)

(define (next-player)
  (if (not (= 0 (length players)))
    (begin
      (set! active-index (modulo (+ active-index 1) (length players)))
      (display "next player\n")
    )
    (display "no players!\n")
  )
)
(define (active-player-id)
  (cdr (list-ref players active-index))
)


(define (move-player id pos)
  (gameobj-setpos-relxz! (gameobj-by-id id) pos)
)

;(define (move-player player keyVec2 mouseVec2)
;  (assoc-ref players player)
;  (display "move placeholder")
;)


;(define (onMouseMove)
;  (display "\n")
;)
;(define (get-key-vec2)
;  '(0 0)
;)
;(define (get-key-vec3)
;  '(0 0 0)
;)

;(define (get-player1)
;  (list (list 0 0) (list 0 0 0))
;)

(define (onFrame)
  ;(let ((playerinput (get-player1)))
  ;  (move-player (car playerinput) (cadr playerinput))
  ;)
  0
)

(define (dir-from-key key) '(0.1 0 0))

(define (onKey key scancode action mods)
  (if (not (= active-index -1))
    (move-player (active-player-id) (dir-from-key key))
  )

  (if (and (= key 257) (= action 1))
    (send-tcp "test message 1")
  )
  (if (and (= key 67) (= action 1))
    (connect-server (car (list-servers)))
  )
  (if (and (= key 77) (= action 1))
    (next-player)
  )
)


