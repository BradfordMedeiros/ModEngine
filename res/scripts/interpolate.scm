
(define frompos  (list 0  0 0))
(define topos    (list 10 0 0))
(define interpos (lerp frompos topos 0.4))
(define expectedPos (list 4  0 0))

(define fromquat (orientation-from-pos (list 0 0 0) (list -1 0 0)))
(define toquat (orientation-from-pos (list 0 0 0) (list 1 0 0)))

(define interquat (move-relative (list 0 0 0) (slerp fromquat toquat 0.5) 1))
(define expectedQuat (list 0 0 -1))
(define interquat2 (move-relative (list 0 0 0) (slerp fromquat toquat 0.25) 1))
(define expectedQuat2 (list -0.707 0 -0.707))  ; approximately, more or less, technically should be length 1, but direction important here


(define (onKeyChar char) 
  (format #t "interpos is: ~a, expected ~a\n" interpos expectedPos)
  (format #t "interquat is: ~a, expected ~a\n" interquat expectedQuat)
  (format #t "interquat2 is: ~a, expected ~a\n" interquat2 expectedQuat2)
)
