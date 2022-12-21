
(define boxObj (lsobj-name "box"))
(define boxId (gameobj-id boxObj))
(define followerObj (lsobj-name "follower"))
(define followerId (gameobj-id followerObj))
(define followerObj2 (lsobj-name "follower2"))
(define followerId2 (gameobj-id followerObj2))

(make-parent followerId2 followerId)
(make-parent followerId boxId)

;(define (onKey key scancode action mods)
;  (if (and (equal? key 257) (equal? action 1))
;  	(begin
;			(format #t "box: ~a, follower: ~a, boxid: ~a, followerid: ~a\n" boxObj followerObj boxId followerId)
;			(parent)
;  	)
;  )
;)

